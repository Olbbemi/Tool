template<typename type>
class List
{
private:
	struct Node
	{
		type data;
		Node *Llink, *Rlink;
	};

	Node Head, Tail;
	int cnt;

public:
	class Iterator
	{
	private:
		Node *ptr;

	public:
		friend void List::clear();
		friend void List::insert(Iterator &itor, type data);
		friend Iterator List::erase(Iterator &itor);

		Iterator(Node *node = NULL)
		{
			ptr = node;
		}

		Iterator operator ++(int)
		{
			Node *temp = ptr;
			ptr = ptr->Rlink;
			return temp;
		}

		Iterator operator ++()
		{
			return ptr = ptr->Rlink;
		}

		Iterator operator --(int)
		{
			Node *temp = ptr;
			ptr = ptr->Llink;
			return temp;
		}

		Iterator operator --()
		{
			return ptr = ptr->Llink;
		}

		type& operator *()
		{
			return ptr->data;
		}

		bool operator ==(const Iterator &value)
		{
			if (this->ptr == value.ptr)
				return true;
			else
				return false;
		}

		bool operator !=(const Iterator &value)
		{
			if (this->ptr == value.ptr)
				return false;
			else
				return true;
		}
	};

	List()
	{
		cnt = 0;
		Head.data = Tail.data = 0;
		Head.Llink = NULL;	Head.Rlink = &Tail;
		Tail.Rlink = NULL;  Tail.Llink = &Head;
	}

	type front()
	{
		return Head.Rlink->data;
	}

	type back()
	{
		return Tail.Llink->data;
	}

	void push_back(type data)
	{
		Node *p = new Node;
		p->data = data;
		p->Llink = Tail.Llink;
		p->Rlink = &Tail;

		Tail.Llink->Rlink = p;
		Tail.Llink = p;

		cnt++;
	}

	void push_front(type data)
	{
		Node *p = new Node;
		p->data = data;
		p->Rlink = Head.Rlink;
		p->Llink = &Head;

		Head.Rlink->Llink = p;
		Head.Rlink = p;

		cnt++;
	}

	void pop_back()
	{
		Node *garbage = Tail.Llink;
		Tail.Llink = garbage->Llink;

		garbage->Llink->Rlink = &Tail;
		garbage->Llink = garbage->Rlink = NULL;
		delete garbage;

		cnt--;
	}

	void pop_front()
	{
		Node *garbage = Head.Rlink;
		Head.Rlink = garbage->Rlink;

		garbage->Rlink->Llink = &Head;
		garbage->Llink = garbage->Rlink = NULL;
		delete garbage;

		cnt--;
	}

	bool empty()
	{
		if (cnt == 0)
			return true;
		else
			return false;
	}

	Node * begin()
	{
		return Head.Rlink;
	}

	Node * end()
	{
		return &Tail;
	}

	void clear()
	{
		List<type>::Iterator itor = List<type>::begin(), temp;
		while (itor != List<type>::end())
		{
			temp = itor++;
			delete temp.ptr;
		}

		Head.Rlink = &Tail;
		Tail.Llink = &Head;
		cnt = 0;
	}

	int size()
	{
		return cnt;
	}

	void insert(Iterator &itor, type data)
	{
		Node *temp = new Node;
		temp->data = data;

		temp->Rlink = itor.ptr->Rlink;
		temp->Llink = itor.ptr;
		itor.ptr->Rlink->Llink = temp;
		itor.ptr->Rlink = temp;

		cnt++;
	}

	Iterator erase(Iterator &itor)
	{
		Iterator garbage = itor++;

		itor.ptr->Rlink->Llink = itor.ptr->Llink;
		itor.ptr->Llink->Rlink = itor.ptr->Rlink;

		delete garbage.ptr;
		return itor;
	}
};