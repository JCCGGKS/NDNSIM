# std::tuple
位于头文件<<tuple>>
## template
template<class... Types> class tuple;
tuple是保存元素对象的集合，每个元素对象可以是不同的类型
## template parameters

# std::pair
位于头文件<<utility>>，是tuple的一个特殊case
## template
template<class T1, class T2>struct pair;

# std::tie
位于头文件<<tuple>>
将参数与元组元素联系起来；std::pair和std::tuple是将两个或者两个以上不同类型的元素打包packing到一个容器中。
而std::tie是将容器中的元素解包unpacking到对应类型的各个对象。如果想要忽略元组中的某个元素，可以使用std::ignore。
