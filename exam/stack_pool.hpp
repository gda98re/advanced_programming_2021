#include <vector>
#include <iostream>

//! An iterator class
/*!
  Iterator of the class stack_pool
*/

template <typename T, typename N = std::size_t>
template <typename O> 
 class _iterator{
    using stackpool = typename stack_pool<T,N> 
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
    const reference operator*() const { return pool->value(index); } //serve implementare anche la versione non const?
    pointer operator->() const { return &**this; } //serve implementare la versione non const?
    _iterator operator++() {
    index = pool->next(index);
    return *this;
    }
    _iterator operator++(int) {
      auto tmp = *this;
      ++(*this);
      return tmp;
    }
    friend bool operator==(const _iterator& x, const _iterator& y) {
      return x.index == y.index;
    }
    friend bool operator!=(const _iterator& x, const _iterator& y) {
      return !(x == y);
    }
  };


//! stack_pool class
/*!
  the class implements a pool of multiple stacks
*/

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

  node_t& node(const stack_type x) noexcept { return pool[x-1]; } //perchè noexcept? la x data può essere sbagliata
  const node_t& node(const stack_type x) const noexcept { return pool[x-1]; }

  void init_free_nodes(const stack_type& first, const size_type& last) { //meglio mettere gli input come stack_type o size_type?
    pool.reserve(last);
    for(auto i = first; i < last; ++i )
      pool.emplace_back(i + 1);
    pool.emplace_back(free_nodes);
    free_nodes = first;
  }

  void check_capacity() {
    if(!capacity()) reserve(8);
    if(!empty(free_nodes))
      return;
    else
      reserve(capacity()*2);
  }

  template <typename X>
  stack_type _push(X&& val, const stack_type head) {
      check_capacity();
      auto tmp = free_nodes; //crea una copia di free node
      free_nodes = next(free_nodes); //viene rimosso il primo free node libero, quindi il nuovo free node libero è il next free node
      value(tmp) = std::forward<X>(val); //viene inserito il valore in input nella posizione libera 
      next(tmp) = head; //la nuova testa (tmp) viene agganciata alla vecchia head
      return tmp; //ritorna il valore della nuova testa della stack
  }

  public:

  stack_pool() noexcept = default;
  stack_pool(const stack_pool&) = default;
  stack_pool& operator=(const stack_pool&) = default;
  stack_pool(stack_pool&&) = default;
  stack_pool& operator=(stack_pool&&) = default; 
  ~stack_pool() = default;

  explicit stack_pool(const size_type n) {  reserve(n); } // reserve n nodes in the pool (custom ctor)

  const stack_type new_stack() const { return end(); } // return an empty stack //ci va const oppure no?

  void reserve(const size_type n) { init_free_nodes(capacity()+size_type(1), n); }// reserve n nodes in the pool

  const size_type capacity() const { return pool.capacity(); } // the capacity of the pool

  bool empty(const stack_type& x) const { return x == end(); }; //può essere implementato con degli iterators tipo begin()==end()

  const stack_type end() const noexcept { return stack_type(0); }  //ok stack_type(0) è il costruttore del tipo dati stack_type (interi essenzialmente)

  value_type& value(const stack_type& x) { return node(x).value; } //completato io
  const value_type& value(const stack_type& x) const { return node(x).value; } //completato io

  stack_type& next(const stack_type& x) { return node(x).next; } //comletato io
  const stack_type& next(const stack_type& x) const { return node(x).next; } //completato io

  const stack_type stack_length(const value_type& x) const {
    auto tmp = stack_type(0);
    for( auto i = cbegin(x); i != cend(x); ++i)
      ++tmp;
    return tmp;
  }

  const stack_type n_free_nodes() const { return stack_length(free_nodes); }

  stack_type push(const value_type& val, const stack_type& head) { return _push(val,head); } //l_value push

  stack_type push(value_type&& val, const stack_type& head) { return _push(std::move(val),head); }//r-value push

  stack_type pop(const stack_type& x) {
    if(empty(x)) { std::cerr << "stack underflow" << std::endl; return end(); } //da implementare con errori
    else {
      auto tmp = next(x);
      next(x) = free_nodes; //aggiorna la posizione della testa dei free nodes
      free_nodes = x; // collega la nuova testa dei free_nodes alla vecchia
      return tmp; // ritorna la nuova testa della stack
    }
  } // delete first node

  stack_type free_stack(stack_type& x) {
      while(!empty(x))
      x = pop(x);
      return x;
  } // free entire stack

  friend
  std::ostream& operator<<(std::ostream& os, const stack_pool& x) {
    for( auto i = size_type(1); i <= x.capacity(); ++i)
      os << x.value(i) << " " ;
    os << std::endl;
    return os;
  }

  void print_stack( const stack_type& x) {
    if(x == end()) {std::cout << "stack vuota" << std::endl; return; }
    for(auto i = cbegin(x); i != cend(x); ++i)
      std::cout << *i << "-->" ;
    std::cout << "end" << std::endl;
  }

  using iterator = _iterator<stack_pool<value_type,stack_type>,value_type>; //aggiornato i ...
  using const_iterator = _iterator<const stack_pool<value_type,stack_type>, const value_type>; // aggiornato i ...

  auto begin(const stack_type& x) { return iterator(this,x); }
  auto end(const stack_type& ) { return iterator(this,end()); } // this is not a typo

  auto begin(const stack_type& x) const { return const_iterator(this,x); }
  auto end(const stack_type& ) const { return const_iterator(this,end()); }

  auto cbegin(const stack_type& x) const { return const_iterator(this,x); }
  auto cend(const stack_type& ) const { return const_iterator(this,end()); }
};
