#ifndef USER_TALLY_H
#define USER_TALLY_H

#include "tally.h"

class user_tally
{
    ArrayNDd data_;
public:
    user_tally();

    const ArrayNDd& data() const { return data_; }
};

#endif // USER_TALLY_H
