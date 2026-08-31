// Event handler API + track assembly test (Feature B, B-1).

#include <chrono>
#include <fstream>
#include <iostream>
#include <set>
#include <vector>

#include "mcdriver.h"

#include "cascadeassembler.h"

struct counters
{
    int source = 0, recoil = 0, scatter = 0, ended = 0, other = 0;
};

struct probe
{
    counters c;
    std::set<uint64_t> ids; // distinct source-ion history ids
    int disableAt{ -1 }; // event index at which to turn capture off (-1 = never)
    int seen{ 0 };
    int bumpAt{ -1 }; // source-ion count at which to bump the epoch (-1 = never)
    bool bumped{ false };
    std::set<uint64_t> preBumpIds;
    std::vector<Cascade> cascades;
    CascadeAssembler assembler; // last: joined before cascades/ids on destruction

    probe() : assembler([this](Cascade &&x) { cascades.push_back(std::move(x)); }) { }
};

static void probe_handler(Event ev, const ion &i, void *p)
{
    probe &pr = *static_cast<probe *>(p);
    switch (ev) {
    case Event::NewSourceIon:
        pr.c.source++;
        pr.ids.insert(static_cast<uint64_t>(i.ion_id()));
        if (pr.bumpAt >= 0 && !pr.bumped)
            pr.preBumpIds.insert(static_cast<uint64_t>(i.ion_id()));
        break;
    case Event::NewRecoil:
        pr.c.recoil++;
        break;
    case Event::Scattering:
        pr.c.scatter++;
        break;
    case Event::IonStop:
    case Event::IonExit:
    case Event::Replacement:
        pr.c.ended++;
        break;
    default:
        pr.c.other++;
        break;
    }
    pr.assembler.feed(ev, i);
    if (pr.disableAt >= 0 && ++pr.seen == pr.disableAt)
        pr.assembler.setCapturing(false); // turn capture off mid-run
    if (pr.bumpAt >= 0 && !pr.bumped && pr.c.source >= pr.bumpAt) {
        pr.assembler.setFilterEpoch(1);
        pr.bumped = true;
    }
}

static std::shared_ptr<mcdriver> make_driver(size_t nions, int nthreads)
{
    mcconfig cfg;
    std::ifstream is(CONFIG_JSON);
    if (!is.is_open()) {
        std::cerr << "cannot open " << CONFIG_JSON << std::endl;
        return nullptr;
    }
    if (cfg.parseJSON(is, true, &std::cerr) != 0)
        return nullptr;
    cfg.Run.max_no_ions = nions;
    cfg.Run.threads = nthreads;
    return mcdriver::create(cfg, &std::cerr);
}

#define CHECK(cond)                                                                  \
    do {                                                                             \
        if (!(cond)) {                                                               \
            std::cerr << "FAILED: " #cond " (line " << __LINE__ << ")" << std::endl; \
            return 1;                                                                \
        }                                                                            \
    } while (0)

static int check_probe(const char *tag, probe &pr)
{
    pr.assembler.flush();

    size_t tracks = 0, verts = 0;
    std::set<uint64_t> cids;
    for (const auto &cc : pr.cascades) {
        CHECK(!cc.buff.empty());
        CHECK(cc.start_pos.size() == cc.length.size());
        CHECK(!cc.start_pos.empty());
        int32_t prev_end = 0;
        for (size_t k = 0; k < cc.start_pos.size(); ++k) {
            CHECK(cc.length[k] > 0);
            CHECK(cc.start_pos[k] == prev_end); // tracks tile buff with no gaps
            prev_end = cc.start_pos[k] + cc.length[k];
        }
        CHECK(static_cast<size_t>(prev_end) == cc.buff.size());
        CHECK(cc.buff[cc.start_pos[0]].rid == 0); // first track is the source ion
        cids.insert(cc.id);
        tracks += cc.start_pos.size();
        verts += cc.buff.size();
    }
    std::cout << tag << ": source=" << pr.c.source << " recoil=" << pr.c.recoil
              << " scatter=" << pr.c.scatter << " end=" << pr.c.ended << " other=" << pr.c.other
              << " | histories=" << pr.ids.size() << " cascades=" << pr.cascades.size()
              << " tracks=" << tracks << " verts=" << verts << std::endl;

    CHECK(pr.c.source > 0);
    CHECK(pr.c.recoil > 0);
    CHECK(pr.c.scatter > 0);
    CHECK(pr.c.other == 0);
    CHECK(pr.c.source + pr.c.recoil == pr.c.ended);
    CHECK(pr.assembler.dropped() == 0); // consumer kept up: no events dropped
    CHECK(cids.size() == pr.cascades.size()); // cascade ids are unique
    CHECK(pr.cascades.size() == pr.ids.size()); // one cascade per source-ion history
    CHECK(tracks == (size_t)(pr.c.source + pr.c.recoil));
    CHECK(verts > 0);
    return 0;
}

int main()
{
    // 1. single thread
    {
        auto D = make_driver(30, 1);
        CHECK(D);
        probe pr;
        pr.assembler.setCapturing(true);
        CHECK(D->install_event_handler(probe_handler, CascadeAssembler::eventMask(), &pr, 0));
        D->exec(nullptr, 200);
        if (check_probe("single", pr))
            return 1;
    }

    // 2. no handler: the run must be untouched
    {
        auto D = make_driver(30, 1);
        CHECK(D);
        D->exec(nullptr, 200);
        CHECK(!D->run_history().empty());
        size_t ran = D->run_history().back().total_ion_count;
        std::cout << "transparency: requested=30 ran=" << ran << std::endl;
        CHECK(ran > 0);
    }

    // 3. 4 threads, more ions
    {
        auto D = make_driver(200, 4);
        CHECK(D);
        probe pr;
        pr.assembler.setCapturing(true);
        CHECK(D->install_event_handler(probe_handler, CascadeAssembler::eventMask(), &pr, 0));
        D->exec(nullptr, 200);
        if (check_probe("stress", pr))
            return 1;
    }

    // 4. recording overhead: the same run with capture off vs on
    {
        int Nions = 1000; // enough to measure ms
        std::cout << "Testing capture overhead: " << Nions << " ions, 1 thread" << std::endl;

        // case 0 : no handler at all
        std::cout << "  running without handler: ...";
        std::cout.flush();
        auto D00 = make_driver(Nions, 1);
        CHECK(D00);
        D00->exec(nullptr, 200);
        double ms_none = D00->run_history().back().cpu_time_s * 1000.0 / Nions; // convert to ms/ion
        std::cout << "done." << std::endl;

        std::cout << "  running with handler, capture off: ...";
        std::cout.flush();
        probe off; // capture stays off (default)
        auto D0 = make_driver(Nions, 1);
        CHECK(D0);
        CHECK(D0->install_event_handler(probe_handler, CascadeAssembler::eventMask(), &off, 0));
        D0->exec(nullptr, 200);
        double ms_off = D0->run_history().back().cpu_time_s * 1000.0 / Nions;
        std::cout << "done." << std::endl;

        std::cout << "  running with handler, capture on: ...";
        std::cout.flush();
        probe on;
        on.assembler.setCapturing(true);
        auto D1 = make_driver(Nions, 1);
        CHECK(D1);
        CHECK(D1->install_event_handler(probe_handler, CascadeAssembler::eventMask(), &on, 0));
        D1->exec(nullptr, 200);
        double ms_on = D1->run_history().back().cpu_time_s * 1000.0 / Nions;
        std::cout << "done." << std::endl;

        off.assembler.flush();
        on.assembler.flush();

        double off_overhead = int((ms_off - ms_none) / ms_none * 1000) / 10.0; // round to 0.1%
        double on_overhead = int((ms_on - ms_none) / ms_none * 1000) / 10.0; // round to 0.1%
        std::cout << "timing(" << Nions << " ions):" << std::endl;
        std::cout << "  no handler:  " << ms_none << " ms/ion" << std::endl;
        std::cout << "  capture off: " << ms_off << " ms/ion, overhead=" << off_overhead << "%"
                  << std::endl;
        std::cout << "  capture on:  " << ms_on << " ms/ion, overhead=" << on_overhead << "%"
                  << std::endl;

        CHECK(off.cascades.empty()); // capture off: nothing recorded
        CHECK(!on.cascades.empty()); // capture on: cascades recorded
    }

    // 5. capture disabled mid-run: flush must not emit a partial cascade
    {
        probe pr;
        pr.disableAt = 100; // turn capture off inside the first cascade
        pr.assembler.setCapturing(true);
        auto D = make_driver(50, 1);
        CHECK(D);
        CHECK(D->install_event_handler(probe_handler, CascadeAssembler::eventMask(), &pr, 0));
        D->exec(nullptr, 200);
        pr.assembler.flush();
        std::cout << "capture-off: cascades=" << pr.cascades.size() << std::endl;
        CHECK(pr.cascades.empty()); // interrupted cascade discarded, not emitted
    }

    // 6. limits filter at the source: dropped tracks never stored, source ion kept
    {
        size_t tracks_ref = 0;
        {
            probe ref;
            ref.assembler.setCapturing(true);
            auto D = make_driver(50, 1);
            CHECK(D);
            CHECK(D->install_event_handler(probe_handler, CascadeAssembler::eventMask(), &ref, 0));
            D->exec(nullptr, 200);
            ref.assembler.flush();
            for (const auto &cc : ref.cascades)
                tracks_ref += cc.start_pos.size();
        }

        probe g; // generation cutoff
        g.assembler.setCapturing(true);
        g.assembler.setGenCutoff(1);
        g.assembler.setFilterEpoch(7);
        auto Dg = make_driver(50, 1);
        CHECK(Dg);
        CHECK(Dg->install_event_handler(probe_handler, CascadeAssembler::eventMask(), &g, 0));
        Dg->exec(nullptr, 200);
        g.assembler.flush();
        size_t tracks_g = 0;
        for (const auto &cc : g.cascades) {
            CHECK(cc.epoch == 7);
            CHECK(cc.buff[cc.start_pos[0]].rid == 0); // source ion kept
            for (size_t k = 0; k < cc.start_pos.size(); ++k)
                CHECK(cc.buff[cc.start_pos[k]].rid <= 1);
            tracks_g += cc.start_pos.size();
        }
        CHECK(!g.cascades.empty());
        CHECK(tracks_g < tracks_ref); // deeper generations dropped

        probe e; // energy threshold
        e.assembler.setCapturing(true);
        e.assembler.setEnergyThreshold(1000.f);
        auto De = make_driver(50, 1);
        CHECK(De);
        CHECK(De->install_event_handler(probe_handler, CascadeAssembler::eventMask(), &e, 0));
        De->exec(nullptr, 200);
        e.assembler.flush();
        for (const auto &cc : e.cascades) {
            CHECK(cc.buff[cc.start_pos[0]].rid == 0); // source ion kept
            for (size_t k = 0; k < cc.start_pos.size(); ++k) {
                const TrackVertex &v = cc.buff[cc.start_pos[k]];
                if (v.rid > 0)
                    CHECK(v.energy >= 1000.f); // recoils start at/above the threshold
            }
        }
        CHECK(!e.cascades.empty());
        std::cout << "limits: ref_tracks=" << tracks_ref << " gen<=1_tracks=" << tracks_g
                  << " e>=1keV cascades=" << e.cascades.size() << std::endl;
    }

    // 7. filter change mid-capture: cascades captured before it keep the old epoch
    //    (recorder rejects them), later ones get the new one (capture-time stamp)
    {
        probe pr;
        pr.assembler.setCapturing(true);
        pr.bumpAt = 5; // bump after the 5th source ion
        auto D = make_driver(50, 1);
        CHECK(D);
        CHECK(D->install_event_handler(probe_handler, CascadeAssembler::eventMask(), &pr, 0));
        D->exec(nullptr, 200);
        pr.assembler.flush();
        CHECK(pr.bumped);
        bool sawNew = false;
        for (const auto &cc : pr.cascades) {
            CHECK(cc.epoch == 0 || cc.epoch == 1);
            if (pr.preBumpIds.count(cc.id))
                CHECK(cc.epoch == 0); // pre-change capture never gets the new epoch
            if (cc.epoch == 1)
                sawNew = true;
        }
        CHECK(sawNew); // post-change captures appear
        std::cout << "active-change: cascades=" << pr.cascades.size()
                  << " pre=" << pr.preBumpIds.size() << std::endl;
    }

    std::cout << "ALL PASS" << std::endl;
    return 0;
}
