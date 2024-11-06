#ifndef EVENT_STREAM_H
#define EVENT_STREAM_H

#include <cassert>
#include <cstring>
#include <string>
#include <vector>
#include <fstream>

class event_stream;
class ion;

/**
 * @brief The event class stores data for a given Monte-Carlo event.
 * 
 * The data is a vector of 32-bit float numbers.
 * 
 * The vector size and the meaning of each element depends on the event type.
 * E.g., for PKA events we may have the energy, position and direction of the PKA recoil
 * and other data.
 * 
 * columnNames() and columnDescriptions() give information on the  
 * of the event data.
 * 
 * @ingroup Tallies
 * 
 */
class event
{
protected:
    std::vector<float> buff_;
    std::vector<std::string> columnNames_;
    std::vector<std::string> columnDescriptions_;
public:
    /// @brief Create empty event
    event()
    {}
    /// @brief Create an event of size n with given column names and descriptions
    event(size_t n,
          const std::vector<std::string> &names,
          const std::vector<std::string> &descs) : 
          buff_(n),
          columnNames_(names),
          columnDescriptions_(descs)
    {}
    /// @brief Copy constructor
    event(const event& ev) :
    buff_(ev.buff_), 
    columnNames_(ev.columnNames_),
    columnDescriptions_(ev.columnDescriptions_)
    {}
    virtual ~event()
    {}
    /// zero-out event data
    void reset() {
        if (!buff_.empty())
            std::memset(buff_.data(),0,buff_.size()*sizeof(float));
    }
    /// Return the event size (# of float values)
    size_t size() const { return buff_.size(); }
    /// Return a pointer to the event data
    const float* data() const { return buff_.data(); }
    /// Return the names of the individual event columns
    const std::vector<std::string>& columnNames() const
    { return columnNames_;}
    /// Return the descriptions of the individual event columns
    const std::vector<std::string>& columnDescriptions() const
    { return columnDescriptions_;}
};

/**
 * @brief A class representing a stream of MC events
 * 
 * An event_stream can store \ref event objects of a given type.
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
    std::ofstream fs_;
    std::string fname_;
    event event_proto_;

public:
    /// Create an empty event_stream
    event_stream() :
        rows_(0), cols_(0)
    {}
    /// Close the file, remove data and destroy the event_stream object
    virtual ~event_stream()
    {
        close();
        remove();
    }
    /// Open event_stream with given file name and event type
    int open(const std::string& fname, const event& ev);
    /// Count of events stored in the stream (rows)
    size_t rows() const { return rows_; }
    /// Size of each event (columns)
    size_t cols() const { return cols_; }
    /// Name of the disk file holding the event data
    const std::string& fileName() const { return fname_; }
    /// @brief Close the event stream
    void close();
    void remove();
    /// Write an event to the stream 
    void write(const event* ev);
    /// Merge data from another stream into this one 
    int merge(const event_stream& ev);
    /// Return true if the stream is open 
    bool is_open() const { return fs_.is_open(); }
    /// Returns a refence to the event prototype currently saved in the stream
    const event& event_prototype() const { return event_proto_; }
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
 * - # of vacancies, replacements and interstitials generated in this PKA cascade
 * 
 * The PKA event buffer is created together with a PKA and lives throught 
 * the PKA cascade, accumulating data.  Thus, the damage energy and 
 * total numbers of cascade defects are obtained.
 * 
 * @ingroup Tallies
 * 
 */
class pka_event : public event
{
    int natoms_;

    enum offset_t {
        ofIonId = 0,
        ofAtomId = 1,
        ofCellId = 2,
        ofErg = 3,
        ofTdam = 4,
        ofVac = 5
    };

public:

    pka_event() :
        event(),
        natoms_(0)
    {}

    /**
     * @brief Set the number of atoms in the target
     * 
     * For each target atom, 3 columns are added to store
     * vacancies, replacements and interstitials
     * 
     * @param n number of atoms in the target (excluding the projectile)
     * @param labels atom labels
     */
    void setNatoms(int n, const std::vector<std::string>& labels);
    /// Initialize the event buffer for PKA ion i 
    void init(const ion* i);
    
    int ionid() const { return buff_[ofIonId]; }
    int atomid() const { return buff_[ofAtomId]; }
    int cellid() const { return buff_[ofCellId]; }
    float recoilE() const { return buff_[ofErg]; }


    float& Tdam() { return buff_[ofTdam]; }
    const float& Tdam() const { return buff_[ofTdam]; }
    float& Vac(int atom_id) { return buff_[ofVac + atom_id]; }
    const float& Vac(int atom_id) const { return buff_[ofVac + atom_id]; }
    float& Repl(int atom_id) { return buff_[ofVac + natoms_ + atom_id]; }
    const float& Repl(int atom_id) const { return buff_[ofVac + natoms_ + atom_id]; }
    float& Impl(int atom_id) { return buff_[ofVac + 2*natoms_ + atom_id]; }
    const float& Impl(int atom_id) const { return buff_[ofVac + 2*natoms_ + atom_id]; }
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
class exit_event : public event
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

    exit_event();
    /// Set the event buffer to the data of the given \ref ion
    void set(const ion* i);
};

#endif // EVENT_STREAM_H
