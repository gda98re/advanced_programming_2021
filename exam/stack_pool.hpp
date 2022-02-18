#include <vector>
#include <iostream>


template <typename stackpool, typename O, typename N>
class _iterator;


template <typename T, typename N = std::size_t>
class stack_pool{

  struct node_t{
    T value;
    N next;
    explicit node_t(const N x): value{}, next{x} {} //ci va explicit o può andare la conversione implicita?
  };

  std::vector<node_t> pool;
  using stack_type = N;
  using value_type = T;
  using size_type = typename std::vector<node_t>::size_type;
  stack_type free_nodes{stack_type(0)}; // at the beginning, it is empty

  node_t& node(const stack_type x) noexcept { return pool[x-1]; }
  const node_t& node(const stack_type x) const noexcept { return pool[x-1]; }

  void init_free_nodes(const size_type& first, const size_type& last);

  void check_capacity();

  template <typename X>
  stack_type _push(X&& val, const stack_type head);

  public:

  stack_pool() noexcept = default;
  stack_pool(const stack_pool&) = default;
  stack_pool& operator=(const stack_pool&) = default;
  stack_pool(stack_pool&&) noexcept = default;
  stack_pool& operator=(stack_pool&&) noexcept = default; 
  ~stack_pool() noexcept = default;
  explicit stack_pool(const size_type n) {  reserve(n); } // reserve n nodes in the pool (custom ctor)

  const stack_type new_stack() const noexcept { return end(); } // return an empty stack //ci va const oppure no?

  void reserve(const size_type n) { init_free_nodes(capacity()+size_type(1), n); }// reserve n nodes in the pool

  const size_type capacity() const noexcept { return pool.capacity(); } // the capacity of the pool

  bool empty(const stack_type& x) const noexcept { return x == end(); }; //può essere implementato con degli iterators tipo begin()==end()

  const stack_type end() const { return stack_type(0); }  //stack_type(0) è il costruttore del tipo stack_type (chiedere a Gallo)

  value_type& value(const stack_type& x) noexcept  { return node(x).value; }
  const value_type& value(const stack_type& x) const noexcept  { return node(x).value; }

  stack_type& next(const stack_type& x) noexcept  { return node(x).next; }
  const stack_type& next(const stack_type& x) const noexcept  { return node(x).next; }

  stack_type push(const value_type& val, const stack_type& head) { return _push(val,head); } //l_value push

  stack_type push(value_type&& val, const stack_type& head) { return _push(std::move(val),head); }//r-value push

  stack_type pop(const stack_type& x);

  stack_type free_stack(stack_type& x);

  friend std::ostream& operator<<(std::ostream& os, const stack_pool& x) { //friend serve per accedere a size_type
    for(auto i = size_type(1); i <= x.capacity(); ++i)
      os << x.value(i) << " " ;
    os << std::endl;
    return os;
  }

  using iterator = _iterator<stack_pool<value_type,stack_type>, value_type, stack_type>;
  using const_iterator = _iterator<const stack_pool<value_type,stack_type>, const value_type, const stack_type>;

  iterator begin(const stack_type& x) { return iterator(this,x); }
  iterator end(const stack_type& ) { return iterator(this,end()); } // this is not a typo

  const_iterator begin(const stack_type& x) const { return const_iterator(this,x); }
  const_iterator end(const stack_type& ) const { return const_iterator(this,end()); }

  const_iterator cbegin(const stack_type& x) const { return const_iterator(this,x); }
  const_iterator cend(const stack_type& ) const { return const_iterator(this,end()); }
};


template <typename T, typename N>
void stack_pool<T,N>::init_free_nodes(const size_type& first, const size_type& last) { //meglio mettere gli input come stack_type o size_type?
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
N stack_pool<T,N>::pop(const stack_type& x) {
  if(empty(x)) { std::cerr << "stack underflow" << std::endl; return end(); }
  else {
    auto tmp = next(x); //tmp è la testa della stack
    next(x) = free_nodes; //la nuova testa dei free nodes (x) punta alla vecchia testa dei free nodes (free_nodes)
    free_nodes = x; // la testa dei free nodes viene aggiornata
    return tmp; // ritorna la nuova testa della stack
  }
} // delete first node

template <typename T, typename N>
N stack_pool<T,N>::free_stack(stack_type& x) {
  while(!empty(x))
    x = pop(x);
  return x;
} // free entire stack


template <typename stackpool, typename O, typename N> //è necessario anche il template stackpool, perchè per i const_itertors il puntatore this è const, (il metodo è marcato const) per cui il primo membro deve essere un const stack_pool*
class _iterator{
  stackpool* pool;
  N index;
  public:
  using stack_type = N;
  using value_type = O;
  using reference = value_type&;
  using pointer = value_type*;
  using difference_type = std::ptrdiff_t;
  using iterator_category = std::forward_iterator_tag;

  _iterator(stackpool* const p , const stack_type& x): pool{p}, index{x} {}
  const reference operator*() const noexcept { return pool->value(index); } //serve implementare anche la versione non const?
  pointer operator->() const noexcept { return &**this; } //serve implementare la versione non const?
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
