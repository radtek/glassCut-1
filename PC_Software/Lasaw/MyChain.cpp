#include "stdafx.h"
#include "MyChain.h"


//链表的析构函数，用于删除所有的链表中的节点
template<class T>
CMyChain<T>::~CMyChain(){
	Erase();
}


//清除掉链表中的所有元素并且释放内存
template<class T>
void CMyChain<T>::Erase(){
	ChainNode<T>*next;
	//指向下一个节点
	while (first){
		next = first->link;
		delete first;
		first = next;
	}
}

//确认链表的长度
template<class T>
int CMyChain<T>::Length()const{
	ChainNode<T>*current = first;
	int length = 0;
	while (current){
		length++;
		current = current->link;
	}
	return length;
}
//在链表中查找第k个元素
//存在就储存到x中
//不存在则返回false，否则返回true
template<class T>
bool CMyChain<T>::Find(int k, T&x)const{
	if (k < 1 || first==0)
		return false;
	ChainNode<T>*current = first;
	int index = 1;
	while (index < k){
		current = current->link;
		index++;
	}
	if (current){
		x = current->data;
		return true;
	}
	return false;
}
//在链表中搜索
//查找x，如果发现则返回x的下标
//如果x不存在则返回0
template<class T>
int CMyChain<T>::Search(const T&x)const{
	ChainNode<T>*current = first;
	int index = 1;
	while (current && current->data != x){
		current = current->link;
		index++;
	}
	if (current){
		return index;
	}
	return 0;
}
//从链表中删除一个元素
//将第k个元素取至x
//然后从链表中删除第k个元素
//如果不存在则引发异常OutOfBounds
template<class T>
CMyChain<T>& CMyChain<T>::Delete(int k, T& x){
	if (k < 1 || !first){
		return NULL;
	}
	ChainNode<T>*p = first;
	if (k == 1){
		first = first->link;
	}
	else{
		ChainNode<T>*q = first;
		for (int index = 1; index < k - 1 && q; index++){
			q = q->link;
			//此时q指向要删除的前一个节点
		}
		if (!q || !q->link){
			return NULL;
		}
		p = q->link;
		if (p == last)
			last = q;
		q->link = p->link;
		//从链表中删除该节点
		x = p->data;
		delete p;
		return *this;
	}
}
//在第k个位置之后插入元素
//不存在则报OutOfBounds异常
//没有足够内存则报NoMem异常
template<class T>
CMyChain<T>& CMyChain<T>::Insert(int k, const T&x){
	if (k < 0){
		return NULL;
	}
	ChainNode<T>*p = first;
	for (int index = 1; index < k && p; index++){
		p = p->link;
	}
	if (k > 0 && !p){
		return NULL;
	}
	ChainNode<T>*y = new ChainNode<T>;
	y->data = x;
	if (k){
		y->link = p->link;
		p->link = y;
	}
	else{
		//作为第一个元素插入
		y->link = first;
		first = y;
	}
	if (!y->link)
		last = y;
	return *this;
}


//在链表右端添加一个数据
template<class T>
CMyChain<T>& CMyChain<T>::Append(const T&x){
	ChainNode<T>*y;
	y = new ChainNode<T>;
	y->data = x;
	y->link = 0;
	if (first){
		last->link = y;
		last = y;
	}
	else{
		first = last = y;
	}
	return *this;
}