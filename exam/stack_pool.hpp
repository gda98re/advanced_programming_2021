#pragma once
#include <vector>


template <typename stackpool, typename T, typename N>
class _iterator;


template <typename T, typename N = std::size_t>
class stack_pool{

  struct node_t{
    T value;
    N next;
    explicit node_t(const N x): value{}, next{x} {} //custom ctor usato in init_free_nodes in emplace_back()
  };

  std::vector<node_t> pool;
  using stack_type = N;
  using value_type = T;
  using size_type = typename std::vector<node_t>::size_type;
  stack_type free_nodes{stack_type(0)}; // at the beginning, it is empty

  node_t& node(const stack_type x) noexcept { return pool[x-1]; }
  const node_t& node(const stack_type x) const noexcept { return pool[x-1]; }

  void init_free_nodes(const size_type first, const size_type last);

  void check_capacity();

  template <typename X>
  stack_type _push(X&& val, const stack_type head);

  public:

  stack_pool() noexcept = default; //default ctor
  stack_pool(const stack_pool&) = default; //copy ctor
  stack_pool& operator=(const stack_pool&) = default; //copy assignment
  stack_pool(stack_pool&&) noexcept = default; //move ctor
  stack_pool& operator=(stack_pool&&) noexcept = default; //move assignment
  ~stack_pool() noexcept = default; //dtor
  explicit stack_pool(const size_type n) { reserve(n); } //custom ctor, reserve n nodes in the pool

  stack_type new_stack() const noexcept { return end(); } // return an empty stack

  void reserve(const size_type n) { init_free_nodes(capacity()+1, n); }// reserve n nodes in the pool

  size_type capacity() const noexcept { return pool.capacity(); } // the capacity of the pool

  bool empty(const stack_type x) const noexcept { return x == end(); };

  stack_type end() const noexcept { return stack_type(0); }

  value_type& value(const stack_type x) noexcept  { return node(x).value; }
  const value_type& value(const stack_type x) const noexcept  { return node(x).value; }

  stack_type& next(const stack_type x) noexcept  { return node(x).next; }
  const stack_type& next(const stack_type x) const noexcept  { return node(x).next; }

  stack_type push(const value_type& val, const stack_type head) { return _push(val,head); } //l_value push

  stack_type push(value_type&& val, const stack_type head) { return _push(std::move(val),head); }//r-value push

  stack_type pop(const stack_type x);

  stack_type free_stack(stack_type x);

  using iterator = _iterator<stack_pool, value_type, stack_type>;
  using const_iterator = _iterator<stack_pool, const value_type, stack_type>;

  iterator begin(const stack_type x) { return iterator(this,x); }
  iterator end(const stack_type ) noexcept { return iterator(this,end()); } // this is not a typo

  const_iterator begin(const stack_type x) const { return const_iterator(this,x); }
  const_iterator end(const stack_type ) const noexcept { return const_iterator(this,end()); }

  const_iterator cbegin(const stack_type x) const { return const_iterator(this,x); }
  const_iterator cend(const stack_type ) const noexcept { return const_iterator(this,end()); }
};


template <typename T, typename N>
void stack_pool<T,N>::init_free_nodes(const size_type first, const size_type last) {
  pool.reserve(last);
  for(auto i = first; i < last; ++i )
    pool.emplace_back(i + 1); //costruisco i free nodes nuovi utilizzando il custom ctor di node
  pool.emplace_back(free_nodes); //l'ultimo  free node costruito punta alla vecchia testa dei free_nodes
  free_nodes = first; //setta il valore della testa dei free_nodes
}

template <typename T, typename N>
void stack_pool<T,N>::check_capacity() {
  if(!capacity()) reserve(8);
  if(!empty(free_nodes))
    return;
  else
    reserve(capacity()*2);
}

template <typename T, typename N>
template <typename X>
N stack_pool<T,N>::_push(X&& val, const stack_type head) {
    check_capacity();
    auto tmp = free_nodes; //crea una copia di free_nodes
    free_nodes = next(free_nodes); //la testa dei free nodes viene aggiornata
    value(tmp) = std::forward<X>(val); //viene inserito il nuovo valore nella posizione libera
    next(tmp) = head; //la nuova testa (tmp) viene agganciata alla vecchia testa della stack
    return tmp; //ritorna il valore della nuova testa della stack
}

template <typename T, typename N>
N stack_pool<T,N>::pop(const stack_type x) {
    auto tmp = next(x); //tmp è la testa della stack
    next(x) = free_nodes; //la nuova testa dei free nodes (x) punta alla vecchia testa dei free nodes (free_nodes)
    free_nodes = x; // la testa dei free nodes viene aggiornata
    return tmp; // ritorna la nuova testa della stack
} // delete first node

template <typename T, typename N>
N stack_pool<T,N>::free_stack(stack_type x) {
  while(!empty(x))
    x = pop(x);
  return x;
} // free entire stack


template <typename stackpool, typename T, typename N>
class _iterator{
  stackpool* pool;
  N index;
  public:
  using stack_type = N;
  using value_type = T;
  using reference = value_type&;
  using pointer = value_type*;
  using difference_type = std::ptrdiff_t;
  using iterator_category = std::forward_iterator_tag;

  _iterator(stackpool* p , stack_type x): pool{p}, index{x} {}
  reference operator*() const noexcept { return pool->value(index); }
  pointer operator->() const noexcept { return &**this; }
  _iterator operator++() {
    index = pool->next(index);
    return *this;
  }
  _iterator operator++(int) {
    auto tmp = *this;
    ++(*this);
    return tmp;
  }
  friend bool operator==(const _iterator& x, const _iterator& y) noexcept {
    return x.index == y.index;
  }
  friend bool operator!=(const _iterator& x, const _iterator& y) noexcept {
    return !(x == y);
  }
};
