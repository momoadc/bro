#ifndef BitVector_h
#define BitVector_h

#include <iterator>
#include <vector>
#include "SerialObj.h"

/**
 * A vector of bits.
 */
class BitVector : SerialObj {
public:
  typedef size_t block_type;
  typedef size_t size_type;
  static size_type npos;
  static block_type bits_per_block;

public:
  /**
   * An lvalue proxy for single bits.
   */
  class Reference {
    friend class BitVector;
    Reference(block_type& block, block_type i);

  public:
    Reference& flip();
    operator bool() const;
    bool operator~() const;
    Reference& operator=(bool x);
    Reference& operator=(Reference const& other);
    Reference& operator|=(bool x);
    Reference& operator&=(bool x);
    Reference& operator^=(bool x);
    Reference& operator-=(bool x);

  private:
    void operator&();
    block_type& block_;
    block_type const mask_;
  };

  typedef bool const_reference;

  /**
   * Default-constructs an empty bit vector.
   */
  BitVector();

  /**
   * Constructs a bit vector of a given size.
   * @param size The number of bits.
   * @param value The value for each bit.
   */
  explicit BitVector(size_type size, bool value = false);

  /**
   * Constructs a bit vector from a sequence of blocks.
   */
  template <typename InputIterator>
  BitVector(InputIterator first, InputIterator last)
    {
    bits_.insert(bits_.end(), first, last);
    num_bits_ = bits_.size() * bits_per_block;
    }

  /**
   * Copy-constructs a bit vector.
   * @param other The bit vector to copy.
   */
  BitVector(const BitVector& other);

  /**
   * Assigns another bit vector to this instance.
   * @param other The RHS of the assignment.
   */
  BitVector& operator=(const BitVector& other);

  //
  // Bitwise operations
  //
  BitVector operator~() const;
  BitVector operator<<(size_type n) const;
  BitVector operator>>(size_type n) const;
  BitVector& operator<<=(size_type n);
  BitVector& operator>>=(size_type n);
  BitVector& operator&=(BitVector const& other);
  BitVector& operator|=(BitVector const& other);
  BitVector& operator^=(BitVector const& other);
  BitVector& operator-=(BitVector const& other);
  friend BitVector operator&(BitVector const& x, BitVector const& y);
  friend BitVector operator|(BitVector const& x, BitVector const& y);
  friend BitVector operator^(BitVector const& x, BitVector const& y);
  friend BitVector operator-(BitVector const& x, BitVector const& y);

  //
  // Relational operators
  //
  friend bool operator==(BitVector const& x, BitVector const& y);
  friend bool operator!=(BitVector const& x, BitVector const& y);
  friend bool operator<(BitVector const& x, BitVector const& y);

  //
  // Basic operations
  //
  /** Appends the bits in a sequence of values.
   * @tparam Iterator A forward iterator.
   * @param first An iterator pointing to the first element of the sequence.
   * @param last An iterator pointing to one past the last element of the
   * sequence.
   */
  template <typename ForwardIterator>
  void append(ForwardIterator first, ForwardIterator last)
    {
    if (first == last)
      return;

    block_type excess = extra_bits();
    typename std::iterator_traits<ForwardIterator>::difference_type delta =
      std::distance(first, last);

    bits_.reserve(blocks() + delta);
    if (excess == 0)
      {
      bits_.back() |= (*first << excess);
      do
        {
        block_type b = *first++ >> (bits_per_block - excess);
        bits_.push_back(b | (first == last ? 0 : *first << excess));
        } while (first != last);
      }
    else
      {
      bits_.insert(bits_.end(), first, last);
      }
    num_bits_ += bits_per_block * delta;
    }

  /**
   * Appends the bits in a given block.
   * @param block The block containing bits to append.
   */
  void append(block_type block);

  /** Appends a single bit to the end of the bit vector.
   * @param bit The value of the bit.
   */
  void push_back(bool bit);

  /**
   * Clears all bits in the bitvector.
   */
  void clear();

  /**
   * Resizes the bit vector to a new number of bits.
   * @param n The new number of bits of the bit vector.
   * @param value The bit value of new values, if the vector expands.
   */
  void resize(size_type n, bool value = false);

  /**
   * Sets a bit at a specific position to a given value.
   * @param i The bit position.
   * @param bit The value assigned to position *i*.
   * @return A reference to the bit vector instance.
   */
  BitVector& set(size_type i, bool bit = true);

  /**
   * Sets all bits to 1.
   * @return A reference to the bit vector instance.
   */
  BitVector& set();

  /**
   * Resets a bit at a specific position, i.e., sets it to 0.
   * @param i The bit position.
   * @return A reference to the bit vector instance.
   */
  BitVector& reset(size_type i);

  /**
   * Sets all bits to 0.
   * @return A reference to the bit vector instance.
   */
  BitVector& reset();

  /**
   * Toggles/flips a bit at a specific position.
   * @param i The bit position.
   * @return A reference to the bit vector instance.
   */
  BitVector& flip(size_type i);

  /**
   * Computes the complement.
   * @return A reference to the bit vector instance.
   */
  BitVector& flip();

  /** Retrieves a single bit.
   * @param i The bit position.
   * @return A mutable reference to the bit at position *i*.
   */
  Reference operator[](size_type i);

  /**
   * Retrieves a single bit.
   * @param i The bit position.
   * @return A const-reference to the bit at position *i*.
   */
  const_reference operator[](size_type i) const;

  /**
   * Counts the number of 1-bits in the bit vector. Also known as *population
   * count* or *Hamming weight*.
   * @return The number of bits set to 1.
   */
  size_type count() const;

  /**
   * Retrieves the number of blocks of the underlying storage.
   * @param The number of blocks that represent `size()` bits.
   */
  size_type blocks() const;

  /**
   * Retrieves the number of bits the bitvector consist of.
   * @return The length of the bit vector in bits.
   */
  size_type size() const;

  /**
   * Checks whether the bit vector is empty.
   * @return `true` iff the bitvector has zero length.
   */
  bool empty() const;

  /**
   * Finds the bit position of of the first 1-bit.
   * @return The position of the first bit that equals to one or `npos` if no
   * such bit exists.
   */
  size_type find_first() const;

  /**
   * Finds the next 1-bit from a given starting position.
   *
   * @param i The index where to start looking.
   *
   * @return The position of the first bit that equals to 1 after position
   * *i*  or `npos` if no such bit exists.
   */
  size_type find_next(size_type i) const;

  bool Serialize(SerialInfo* info) const;
  static BitVector* Unserialize(UnserialInfo* info);

protected:
  DECLARE_SERIAL(BitVector);

private:
  /**
   * Computes the block index for a given bit position.
   */
  static size_type block_index(size_type i)
    {
    return i / bits_per_block;
    }

  /**
   * Computes the bit index within a given block for a given bit position.
   */
  static block_type bit_index(size_type i)
    {
    return i % bits_per_block;
    }

  /**
   * Computes the bitmask block to extract a bit a given bit position.
   */
  static block_type bit_mask(size_type i)
    {
    return block_type(1) << bit_index(i);
    }

  /**
   * Computes the number of blocks needed to represent a given number of
   * bits.
   * @param bits the number of bits.
   * @return The number of blocks to represent *bits* number of bits.
   */
  static size_type bits_to_blocks(size_type bits)
    {
    return bits / bits_per_block
      + static_cast<size_type>(bits % bits_per_block != 0);
    }

  /**
   * Computes the bit position first 1-bit in a given block.
   * @param block The block to inspect.
   * @return The bit position where *block* has its first bit set to 1.
   */
  static size_type lowest_bit(block_type block);

  /**
   * Computes the number of excess/unused bits in the bit vector.
   */
  block_type extra_bits() const;

  /**
   * If the number of bits in the vector are not not a multiple of
   * bitvector::bits_per_block, then the last block exhibits unused bits which
   * this function resets.
   */
  void zero_unused_bits();

  /**
   * Looks for the first 1-bit starting at a given position.
   * @param i The block index to start looking.
   * @return The block index of the first 1-bit starting from *i* or
   * `bitvector::npos` if no 1-bit exists.
   */
  size_type find_from(size_type i) const;

  std::vector<block_type> bits_;
  size_type num_bits_;
};

#endif
