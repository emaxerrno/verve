#include "gtest/gtest.h"
#include "lock_manager.h"

TEST(LockManager, ValidRange) {
  verve::LockManager m(0, 100);
  ASSERT_TRUE(m.TryReadLock(0, 100));
  ASSERT_TRUE(m.TryReadLock(0, 50));
  ASSERT_TRUE(m.TryReadLock(50, 100));
  ASSERT_TRUE(m.TryReadLock(20, 80));
  ASSERT_TRUE(m.TryReadLock(0, 1));
  ASSERT_TRUE(m.TryReadLock(1, 2));
  ASSERT_THROW(m.TryReadLock(0, 0), std::out_of_range);
  ASSERT_THROW(m.TryReadLock(1, 1), std::out_of_range);
  ASSERT_THROW(m.TryReadLock(2, 1), std::out_of_range);
  ASSERT_THROW(m.TryReadLock(0, 101), std::out_of_range);
  ASSERT_THROW(m.TryReadLock(50, 101), std::out_of_range);

  verve::LockManager m2(1, 100);
  ASSERT_TRUE(m2.TryReadLock(1, 100));
  ASSERT_TRUE(m2.TryReadLock(1, 50));
  ASSERT_TRUE(m2.TryReadLock(50, 100));
  ASSERT_TRUE(m2.TryReadLock(20, 80));
  ASSERT_TRUE(m2.TryReadLock(1, 2));
  ASSERT_TRUE(m2.TryReadLock(2, 3));
  ASSERT_THROW(m2.TryReadLock(0, 0), std::out_of_range);
  ASSERT_THROW(m2.TryReadLock(1, 1), std::out_of_range);
  ASSERT_THROW(m2.TryReadLock(2, 2), std::out_of_range);
  ASSERT_THROW(m2.TryReadLock(2, 1), std::out_of_range);
  ASSERT_THROW(m2.TryReadLock(0, 50), std::out_of_range);
  ASSERT_THROW(m2.TryReadLock(0, 100), std::out_of_range);
  ASSERT_THROW(m2.TryReadLock(50, 101), std::out_of_range);
}
