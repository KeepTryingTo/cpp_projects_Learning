#include <iostream>
#include <algorithm>
#include <vector>
#include <list>
#include <future>

//快速排序的原始版本
template<typename	T>
 std::list<T>	sequential_quick_sort(std::list<T>	input)
 {
    if(input.empty())
    {
        return	input;
    }
    std::list<T> result;
    //将元素划分之后放到results中
    result.splice(result.begin(),input , input.begin());		//	1
    //获取开头的元素作为基准元素
    T const& pivot = *result.begin();		//	2
    //开始以基准元素进行划分，返回划分点
    auto divide_point = std::partition(input.begin(),input.end(),
                                            [&](T	const&	t){return	t<pivot;});		//	3
    std::list<T> lower_part;
    //根据划分点得到小于基准元素部分
    lower_part.splice(lower_part.end(),input,input.begin(),divide_point);		//	4
    //递归操作
    auto new_lower(sequential_quick_sort(std::move(lower_part)));		//	5
    auto new_higher(sequential_quick_sort(std::move(input)));		//	6

    //将划分的元素放到指定位置
    result.splice(result.end(),new_higher);		//	7
    result.splice(result.begin(),new_lower);		//	8
    return	result;
 }

 //期望版本的并行

 template<typename	T>
 std::list<T>	parallel_quick_sort(std::list<T>	input)
 {
    if(input.empty())
    {
        return input;
    }
    std::list<T>result;
    result.splice(result.begin(),input,input.begin());
    T const& pivot=*result.begin();
    auto divide_point=std::partition(input.begin(),input.end(),
                                                            [&](T	const&	t){return	t<pivot;});
    std::list<T>lower_part;
    lower_part.splice(lower_part.end(),input,input.begin(),divide_point);
    //注意async和thread的区别是async会返回值
    std::future<std::list<T>>new_lower(std::async(&parallel_quick_sort<T>,std::move(lower_part)));
    auto new_higher(parallel_quick_sort(std::move(input)));		//	2
    result.splice(result.end(),new_higher);		//	3
    result.splice(result.begin(),new_lower.get());		//	4
    return	result;
 }

int main(){
    std::list<int>ls{4,6,7,8,9,3,4,5};
    std::list<int>result = parallel_quick_sort(ls);
    for(auto it = result.begin(); it != result.end(); it++){
        std::cout<<*it<<std::endl;
    }

    return 0;
}