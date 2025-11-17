#ifndef EVENT_STREAM_H
#define EVENT_STREAM_H

#include "geometry.h"
#include <cassert>
#include <cstring>
#include <string>
#include <vector>

class event_stream;
class ion;
class tally;
class material;
class abstract_cascade;

/**
 * @brief The event_buffer class stores data for a given Monte-Carlo event_buffer.
 *
 * The data is a vector of 32-bit float numbers.
 *
 * The vector size and the meaning of each element depends on the event_buffer type.
 * E.g., for PKA events we may have the energy, position and direction of the PKA recoil
 * and other data.
 *
 * columnNames() and columnDescriptions() give information on the
 * of the event_buffer data.
 *
 * @ingroup Tallies
 *
 */
class event_buffer
{
protected:
    uint32_t mask_{ 0 };
    std::vector<float> buff_;
    std::vector<std::string> columnNames_;
    std::vector<std::string> columnDescriptions_;

public:
    /// @brief Create empty event_buffer
    explicit event_buffer(uint32_t m = 0) : mask_(m) { }
    /// @brief Create an event_buffer of size n with given column names and descriptions
    event_buffer(uint32_t m, size_t n, const std::vector<std::string> &names,
                 const std::vector<std::string> &descs)
        : mask_(m), buff_(n), columnNames_(names), columnDescriptions_(descs)
    {
    }
    virtual ~event_buffer() { }
    /// zero-out event_buffer data
    void reset()
    {
        if (!buff_.empty())
            std::memset(buff_.data(), 0, buff_.size() * sizeof(float));
    }
    /// A bit mask for the accepted event types
    uint32_t mask() const { return mask_; }
    /// Return the event_buffer size (# of float values)
    size_t size() const { return buff_.size(); }
    /// Return a pointer to the event_buffer data
    const float *data() const { return buff_.data(); }
    /// Return the names of the individual event_buffer columns
    const std::vector<std::string> &columnNames() const { return columnNames_; }
    /// Return the descriptions of the individual event_buffer columns
    const std::vector<std::string> &columnDescriptions() const { return columnDescriptions_; }
};

/**
 * @brief A class representing a stream of simulation events
 *
 * An event_stream can store \ref event_buffer objects of a given type.
 *
 * During the simulation, events are stored in temporary disk buffers.
 * At the end of the simulation, event data are transfered to the
 * \ref out_file "HDF5 output file" and stored in compressed datasets.
 *
 * The data is actually a 2D array with columns equal to the event size
 * and rows equal to the total number of events.
 *
 * Event column names and descriptions are also stored in the output file.
 *
 */
class event_stream
{
protected:
    size_t rows_, cols_;
    std::FILE *fs_;
    std::string fname_;
    event_buffer event_proto_;

public:
    /// Create an empty event_stream
    event_stream() : rows_(0), cols_(0), fs_(NULL) { }
    /// Close the file, remove data and destroy the event_stream object
    virtual ~event_stream() { close_(); }
    /// Open the event_stream
    int open();
    /// Count of events stored in the stream (rows)
    size_t rows() const { return rows_; }
    /// Size of each event (columns)
    size_t cols() const { return cols_; }
    /// Write an event to the stream
    void write(const event_buffer *ev);
    /// Merge data from another stream into this one
    int merge(event_stream &ev);
    /// Return true if the stream is open
    bool is_open() const { return fs_ != NULL; }
    /// Set the event_buffer prototype. The stream must be in the closed state;
    bool set_event_prototype(const event_buffer &ev);
    /// Returns a refence to the event_buffer prototype currently saved in the stream
    const event_buffer &event_prototype() const { return event_proto_; }
    /// A bit mask for the accepted event types
    uint32_t mask() const { return event_proto_.mask(); }

    void rewind();
    void clear();
    size_t read(float *buff, size_t nevents);
    size_t write(const float *buff, size_t nevents);

private:
    /// @brief Close the event stream
    void close_();
};

/**
 * @brief A class for storing data of a PKA event
 *
 * The following data is stored:
 * - history id
 * - atom id of the PKA recoil
 * - cell id where the PKA was generated
 * - recoil energy
 * - damage energy
 * - # of vacancies, interstitials generated in this PKA cascade
 * - # of recombinations (+correlated recombinations)
 *
 * The PKA event_buffer buffer is created together with a PKA and lives throught
 * the PKA cascade, accumulating data.  Thus, the damage energy and
 * total numbers of cascade defects are obtained.
 *
 * @ingroup Tallies
 *
 */
class pka_buffer : public event_buffer
{
    // # of atoms in the simulation
    int natoms_;

    // buffer offset of various quantities
    enum offset_t {
        ofIonId = 0, // id of the ion that generated the pka
        ofAtomId = 1, // atom id of pka
        ofPos = 2, // position (x,y,z) where pka was created
        ofErg = 5, // pka recoil energy
        ofTdam = 6, // pka damage energy
        ofVac = 7 // vacancies of 1st atom id
    };
    /*
     * After ofVac we have 4*natoms_ memory locations for:
     * Vacancies, Intersitials, Recombinations, Correlated Recomb.
     * for each atom id
     */
    constexpr static int atom_cols_ = 4;

    float Tdam_LSS_, NRT_LSS_, NRT_;

public:
    pka_buffer();

    /**
     * @brief Set the number of atoms in the target
     *
     * For each target atom, 3 columns are added to store
     * vacancies, replacements and interstitials
     *
     * @param n number of atoms in the target (excluding the projectile)
     * @param labels atom labels
     */
    void setNatoms(int n, const std::vector<std::string> &labels);

    /// Initialize the event buffer for PKA ion i
    void init(const ion *i);

    int ionid() const { return buff_[ofIonId]; }
    int atomid() const { return buff_[ofAtomId]; }
    float recoilE() const { return buff_[ofErg]; }

    float &Tdam() { return buff_[ofTdam]; }
    const float &Tdam() const { return buff_[ofTdam]; }
    float &Vac(int atom_id) { return buff_[ofVac + atom_id]; }
    const float &Vac(int atom_id) const { return buff_[ofVac + atom_id]; }
    float &Impl(int atom_id) { return buff_[ofVac + natoms_ + atom_id]; }
    const float &Impl(int atom_id) const { return buff_[ofVac + natoms_ + atom_id]; }
    float &Icr(int atom_id) { return buff_[ofVac + 2 * natoms_ + atom_id]; }
    const float &Icr(int atom_id) const { return buff_[ofVac + 2 * natoms_ + atom_id]; }
    float &Icr_corr(int atom_id) { return buff_[ofVac + 3 * natoms_ + atom_id]; }
    const float &Icr_corr(int atom_id) const { return buff_[ofVac + 3 * natoms_ + atom_id]; }

    // calc NRT values
    void calc_nrt(const ion &i, const material *m);
    const float &Tdam_LSS() const { return Tdam_LSS_; }
    const float &NRT_LSS() const { return NRT_LSS_; }
    const float &NRT() const { return NRT_; }
};

/**
 * @brief A class for storing data for an ion leaving the simulation
 *
 * The following data is stored:
 * - history id
 * - id of the exiting atom
 * - cell id where the atom exits from
 * - ion energy
 * - ion position vector (will be at the cell boundary)
 * - ion direction vector
 *
 * @ingroup Tallies
 *
 */
class exit_buffer : public event_buffer
{

    enum offset_t {
        ofIonId = 0,
        ofAtomId = 1,
        ofCellId = 2,
        ofErg = 3,
        ofPos = 4,
        ofDir = 7,
        ofEnd = 10
    };

    // IonId AtomId CellId erg pos(3) dir(3) s

public:
    exit_buffer();
    /// Set the event buffer to the data of the given \ref ion
    void set(const ion *i);
};

/**
 * @brief A class for storing damage generation events
 *
 * Damage events are 2 kinds:
 * - creation of a vacancy (defect id: 0)
 * - creation of an interstitial (defect id: 1)
 *
 * The following data is stored:
 * - history id = id of the Monte Carlo history that created the defect
 * - recoil id = 0 for beam ions, 1 for pka, 2, 3, ... for higher order recoils
 * - atom id = id of the interstitial species or of the atom that occupied the vacant site
 * - Defect type id: vacancy (0) or interstitial (1)
 * - position vector (x,y,z) where the defect is created
 *
 * @ingroup Tallies
 *
 */
class damage_event_buffer : public event_buffer
{
private:
    enum offset_t { ofHid = 0, ofRid = 1, ofIid = 2, ofDid = 3, ofPos = 4, ofEnd = 7 };

public:
    damage_event_buffer();

    /// Set the event buffer to the data of the given \ref ion
    void set(const ion &i);
};
#endif // EVENT_STREAM_H
