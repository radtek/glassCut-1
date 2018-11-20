#pragma once
//节点类，定义了每个节点的存储类型和指针名称
template<class T>
class ChainNode{
public:
	T data;
	ChainNode<T> *link;
};

//链表类，封装了链表操作的相应方法
template<class T>
class CMyChain
{
public:
	CMyChain(){ first = 0; };
	~CMyChain();
	bool IsEmpty()const{ return first == 0; }	//判断是否为空
	int Length()const;						//返回该链表的长度
	bool Find(int k, T&x)const;				//返回第k个元素到x中
	int Search(const T&x)const;				//返回x所在的位置  
	CMyChain<T>& Delete(int k, T& x);			//删除第k个元素并把它返回到x中
	CMyChain<T>& Insert(int k, const T&x);		//在第k个元素之后插入x
	ChainNode<T> *first;					//指向第一个节点的指针
	ChainNode<T> *last;						//指向最后一个节点的指针

	void Erase();
	CMyChain<T>& Append(const T&x);

};

//链表遍历器类实现对链表的遍历
template<class T>
class ChainIterator{
public:
	T* Initialize(const CMyChain<T>&c){
		location = c.first;
		if (location){
			return &location->data;//为何有地址符？
		}
		return 0;
	}

	T* Next(){
		if (!location)
			return 0;
		location = location->link;
		if (location)
			return &location->data;
		return 0;
	}
private:
	ChainNode<T>*location;
};
