NFD/daemon/table/cs-internal.hpp
```cpp
namespace nfd {
namespace cs {

class EntryImpl;

// Table本质上是一个set
// EntryImpl的定义在/NFD/daemon/table/cs-entry-impl.hpp
typedef std::set<EntryImpl> Table;
typedef Table::const_iterator iterator;

} // namespace cs
} // namespace nfd
```
