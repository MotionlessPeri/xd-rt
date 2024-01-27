//
// Created by Frank on 2024/1/26.
//

#ifndef XD_RT_CONCURRENTUTIL_H
#define XD_RT_CONCURRENTUTIL_H
namespace xd {
template <typename SharedLockType>
	requires requires(SharedLockType lock) {
		{
			lock.lock_shared()
		};
		{
			lock.unlock_shared()
		};
	}
class SharedLockGuard {
public:
	explicit SharedLockGuard(SharedLockType& lock) : lock(lock) { this->lock.lock_shared(); }
	~SharedLockGuard() noexcept { lock.unlock_shared(); }
	SharedLockGuard(const SharedLockGuard& other) = delete;
	SharedLockGuard& operator=(const SharedLockGuard& other) = delete;

private:
	SharedLockType& lock;
};
}  // namespace xd
#endif	// XD_RT_CONCURRENTUTIL_H
