```cpp
namespace nfd {

/** \brief finds the last element satisfying a predicate
 *  \tparam It BidirectionalIterator
 *  \tparam Pred UnaryPredicate
 *
 *  \return Iterator to the last element satisfying the condition,
 *          or \p last if no such element is found.
 *
 *  Complexity: at most \p last-first invocations of \p p
 */
template<typename It, typename Pred>
BOOST_CONCEPT_REQUIRES(
  ((boost::BidirectionalIterator<It>))
  ((boost::UnaryPredicate<Pred, typename std::iterator_traits<It>::value_type>)),
  (It)
)
find_last_if(It first, It last, Pred p)
{
  // template<class Iterator> class reverse_iterator;
  std::reverse_iterator<It> firstR(first), lastR(last);
  // 从后往前找
  auto found = std::find_if(lastR, firstR, p);
  return found == firstR ? last : std::prev(found.base());
}

}  // namespace nfd
```
