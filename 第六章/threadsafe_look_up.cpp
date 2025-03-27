#include <list>
#include <map>
#include <vector>
#include <iostream>
#include <algorithm>
#include <hash_map>

template<typename	Key,typename	Value,typename	Hash=std::hash<Key>	>
 class	threadsafe_lookup_table
 {
 private:
    class	bucket_type
    {
        private:
                typedef	std::pair<Key,Value> bucket_value;
                typedef	std::list<bucket_value>	bucket_data;
                typedef	typename bucket_data::iterator bucket_iterator;
                //建立一个双链表
                bucket_data	data;
                //支持并发读取
                mutable	boost::shared_mutex	mutex;		//	1
                //从表中查找元素
                bucket_iterator	find_entry_for(Key	const&	key)	const		//	2
                {
                    return	std::find_if(data.begin(),data.end(),[&](bucket_value	const&	item){
                        return	item.first==key;
                    });
                }
        public:
                //并发读取数据
                Value	value_for(Key	const&	key,Value	const&	default_value)	const
                {
                    boost::shared_lock<boost::shared_mutex>	lock(mutex);		//	3
                    bucket_iterator	const	found_entry=find_entry_for(key);
                    return	(found_entry==data.end())?
                            default_value:found_entry->second;
                }
                //更新表
                void	add_or_update_mapping(Key	const&	key,Value	const&	value)
                {
                    std::unique_lock<boost::shared_mutex>	lock(mutex);		//	4
                    bucket_iterator	const	found_entry=find_entry_for(key);
                    if(found_entry==data.end())
                    {
                        data.push_back(bucket_value(key,value));
                    }
                    else
                    {
                        found_entry->second=value;
                    }
                }
                //从表中删除元素
                void	remove_mapping(Key	const&	key)
                {
                    std::unique_lock<boost::shared_mutex>	lock(mutex);		//	5
                    bucket_iterator	const	found_entry=find_entry_for(key);
                    if(found_entry!=data.end())
                    {
                            data.erase(found_entry);
                    }
                }
    };
    //建立一个桶数组，每一个桶是一个双向链表
    std::vector<std::unique_ptr<bucket_type>> buckets;		//	6
    Hash	hasher;
    //根据hash获得对应的桶
    bucket_type&	get_bucket(Key	const&	key)	const		//	7
    {
        std::size_t	const	bucket_index=hasher(key)%buckets.size();
        return	*buckets[bucket_index];
    }
 public:
		typedef	Key	key_type;
		typedef	Value	mapped_type;
		typedef	Hash	hash_type;
        //构造函数
		threadsafe_lookup_table(unsigned	num_buckets=19,Hash	const&	hasher_=Hash()):
        //对每一个桶进行初始化（也就是对每一个桶建立双向链表）
		buckets(num_buckets),hasher(hasher_)
		{
            for(unsigned i=0;i<num_buckets;++i)
            {
                buckets[i].reset(new bucket_type);
            }
		}
        //删除复制赋值构造函数和赋值运算操作（）
		threadsafe_lookup_table(threadsafe_lookup_table	const&	other)=delete;
		threadsafe_lookup_table&operator=(threadsafe_lookup_table	const&	other)=delete;
        //根据key得到对应的桶链表，然后从桶链表获得对应的值
		Value	value_for(Key	const&	key,Value	const&	default_value=Value())	const
		{
			return	get_bucket(key).value_for(key,default_value);		//	8
		}
        //向桶里面添加元素
		void	add_or_update_mapping(Key	const&	key,Value	const&	value)
		{
            get_bucket(key).add_or_update_mapping(key,value);		//	9
        }
        //从桶里面移除元素
        void	remove_mapping(Key	const&	key)
        {
            get_bucket(key).remove_mapping(key);		//	10
        }
        //查询表
        std::map<Key,Value>	threadsafe_lookup_table::get_map()	const
        {
            std::vector<std::unique_lock<boost::shared_mutex>>	locks;
            //对每一个桶都加上互斥锁
            for(unsigned i=0;i<buckets.size();++i)
            {
                locks.push_back(std::unique_lock<boost::shared_mutex>(buckets[i].mutex));
            }
            std::map<Key,Value>	res;
            for(unsigned i = 0; i < buckets.size();++i)
            {
                for(bucket_iterator	it = buckets[i].data.begin(); it != buckets[i].data.end(); ++it)
                {
                    res.insert(*it);
                }
            }
            return	res;
        }
};