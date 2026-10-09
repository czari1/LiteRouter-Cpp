#include "RoutingTable.hpp"
#include "Exceptions.hpp"
#include "Logger.hpp"

namespace Net {
    size_t RoutingTable::getTableSize() const {
        return m_routes.size();
    }

    void RoutingTable::clearRoutes(){
        m_routes.clear();
    }

    
};