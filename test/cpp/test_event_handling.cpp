// Event handler API + track assembly test (Feature B, B-1).

#include <fstream>
#include <iostream>
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
    std::vector<Cascade> cascades;
    CascadeAssembler assembler;

    probe() : assembler([this](Cascade &&x) { cascades.push_back(std::move(x)); }) { }
};

static void probe_handler(Event ev, const ion &i, void *p)
{
    probe &pr = *static_cast<probe *>(p);
    switch (ev) {
    case Event::NewSourceIon:
        pr.c.source++;
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

#define CHECK(cond)                                                                                 \
    do {                                                                                            \
        if (!(cond)) {                                                                              \
            std::cerr << "FAILED: " #cond " (line " << __LINE__ << ")" << std::endl;                \
            return 1;                                                                               \
        }                                                                                           \
    } while (0)

static int check_probe(const char *tag, probe &pr)
{
    pr.assembler.flush();

    size_t tracks = 0, verts = 0;
    for (const auto &cc : pr.cascades) {
        CHECK(!cc.tracks.empty());
        CHECK(!cc.tracks.front().verts.empty());
        CHECK(cc.tracks.front().verts.front().rid == 0);
        for (const auto &t : cc.tracks) {
            CHECK(!t.verts.empty());
            tracks += 1;
            verts += t.verts.size();
        }
    }
    std::cout << tag << ": source=" << pr.c.source << " recoil=" << pr.c.recoil
              << " scatter=" << pr.c.scatter << " end=" << pr.c.ended << " other=" << pr.c.other
              << " | cascades=" << pr.cascades.size() << " tracks=" << tracks << " verts=" << verts
              << std::endl;

    CHECK(pr.c.source > 0);
    CHECK(pr.c.recoil > 0);
    CHECK(pr.c.scatter > 0);
    CHECK(pr.c.other == 0);
    CHECK(pr.c.source + pr.c.recoil == pr.c.ended);
    CHECK(pr.cascades.size() == (size_t)pr.c.source);
    CHECK(tracks == (size_t)(pr.c.source + pr.c.recoil));
    CHECK(verts > 0);
    return 0;
}

int main()
{
    // single thread
    {
        auto D = make_driver(30, 1);
        CHECK(D);
        probe pr;
        CHECK(D->install_event_handler(probe_handler, CascadeAssembler::eventMask(), &pr, 0));
        D->exec(nullptr, 200);
        if (check_probe("single", pr))
            return 1;
    }

    // no handler: the run must be untouched
    {
        auto D = make_driver(30, 1);
        CHECK(D);
        D->exec(nullptr, 200);
        CHECK(!D->run_history().empty());
        size_t ran = D->run_history().back().total_ion_count;
        std::cout << "transparency: requested=30 ran=" << ran << std::endl;
        CHECK(ran > 0);
    }

    // 4 threads, more ions
    {
        auto D = make_driver(200, 4);
        CHECK(D);
        probe pr;
        CHECK(D->install_event_handler(probe_handler, CascadeAssembler::eventMask(), &pr, 0));
        D->exec(nullptr, 200);
        if (check_probe("stress", pr))
            return 1;
    }

    std::cout << "ALL PASS" << std::endl;
    return 0;
}
