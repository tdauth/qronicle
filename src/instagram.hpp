#ifndef CHRONICLE_INSTAGRAM_HPP
#define CHRONICLE_INSTAGRAM_HPP

#include "facebook.hpp"

namespace qronicle {

class Instagram : public Facebook {
    public:
        virtual QString id() const override;
        
    protected:
        virtual QString messengerName() const override;
};

}

#endif
