#pragma once
#ifdef _KERNEL_MODE
#include <ntddk.h>
#include <ntdef.h>
#include <xtr1common> 
#else
#include <Windows.h>
#include <utility>
#endif
#include  <Intrin.h> 

#define hide call_hider::hider spoof(_AddressOfReturnAddress());
#ifdef _KERNEL_MODE
#define hide_call(ret_type,name) (call_hider::SafeCall<ret_type,std::remove_reference_t<decltype(*name)>>(name))
#else
#define hide_call(name) (call_hider::SafeCall(name))
#endif

#define MAX_FUNC_BUFFERED 100
#define SHELLCODE_GENERATOR_SIZE 500

namespace call_hider
{
#ifdef _KERNEL_MODE
	typedef unsigned __int64  uintptr_t, size_t;
#pragma region std::forward
	template <class _Ty>
	struct remove_reference {
		using type = _Ty;
		using _Const_thru_ref_type = const _Ty;
	};
	template <class _Ty>
	using remove_reference_t = typename remove_reference<_Ty>::type;

	template <class>
	constexpr bool is_lvalue_reference_v = false; // determine whether type argument is an lvalue reference

	template <class _Ty>
	constexpr bool is_lvalue_reference_v<_Ty&> = true;

	template <class _Ty>
	constexpr _Ty&& forward(
		remove_reference_t<_Ty>& _Arg) noexcept { // forward an lvalue as either an lvalue or an rvalue
		return static_cast<_Ty&&>(_Arg);
	}

	template <class _Ty>
	constexpr _Ty&& forward(remove_reference_t<_Ty>&& _Arg) noexcept { // forward an rvalue as an rvalue
		static_assert(!is_lvalue_reference_v<_Ty>, "bad forward call");
		return static_cast<_Ty&&>(_Arg);
	}
#pragma endregion 

#else
	using namespace std;
#endif

}


namespace call_hider
{
	class hider
	{
	public:
		uintptr_t temp = 0;
		const uintptr_t xor_key = 0x7e4a9c8f5b3d2e1f;
		void* ret_addr_in_stack = 0;

		hider(void* addr) :ret_addr_in_stack(addr)
		{
			temp = *(uintptr_t*)ret_addr_in_stack;
			temp ^= xor_key;
			*(uintptr_t*)ret_addr_in_stack = 0;
		}
		~hider()
		{
			temp ^= xor_key;
			*(uintptr_t*)ret_addr_in_stack = temp;
		}
	};

#ifdef _KERNEL_MODE
	__forceinline PVOID LocateShellCode(PVOID func, size_t size = 500)
	{
		void* addr = ExAllocatePoolWithTag(NonPagedPool, size, (ULONG)"File");
		if (!addr)
			return nullptr;
		return memcpy(addr, func, size);
	}
#else
	__forceinline PVOID LocateShellCode(PVOID func, size_t size = SHELLCODE_GENERATOR_SIZE)
	{
		void* addr = VirtualAlloc(NULL, size, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
		if (!addr)
			return nullptr;
		return memcpy(addr, func, size);
	}
#endif

#ifdef _KERNEL_MODE
	template <typename RetType, typename Func, typename ...Args>
	RetType
#else
	template <typename Func, typename ...Args>
	typename std::invoke_result<Func, Args...>::type
#endif
		__declspec(safebuffers)ShellCodeGenerator(Func f, Args&... args)
	{
#ifdef _KERNEL_MODE
		using this_func_type = decltype(ShellCodeGenerator<RetType, Func, Args&...>);
		using return_type = RetType;
#else
		using this_func_type = decltype(ShellCodeGenerator<Func, Args&...>);
		using return_type = typename std::invoke_result<Func, Args...>::type;
#endif
		const uintptr_t xor_key = 0x7e4a9c8f5b3d2e1f;
		void* ret_addr_in_stack = _AddressOfReturnAddress();
		uintptr_t temp = *(uintptr_t*)ret_addr_in_stack;
		temp ^= xor_key;
		*(uintptr_t*)ret_addr_in_stack = 0;

		if constexpr (std::is_same<return_type, void>::value)
		{
			f(args...);
			temp ^= xor_key;
			*(uintptr_t*)ret_addr_in_stack = temp;
		}
		else
		{
			return_type&& ret = f(args...);
			temp ^= xor_key;
			*(uintptr_t*)ret_addr_in_stack = temp;
			return ret;
		}
	}



#ifdef _KERNEL_MODE
	template<typename RetType, class Func>
#else
	template<class Func >
#endif
	class SafeCall
	{

		Func* funcPtr;

	public:
		SafeCall(Func* func) :funcPtr(func) {}


		template<typename... Args>
		__forceinline decltype(auto) operator()(Args&&... args)
		{
			hide;

#ifdef _KERNEL_MODE
			using return_type = RetType;
			using p_shell_code_generator_type = decltype(&ShellCodeGenerator<RetType, Func*, Args...>);
			PVOID self_addr = static_cast<PVOID>(&ShellCodeGenerator<RetType, Func*, Args&&...>);
#else	
			using return_type = typename std::invoke_result<Func, Args...>::type;
			using p_shell_code_generator_type = decltype(&ShellCodeGenerator<Func*, Args...>);
			p_shell_code_generator_type self_addr = static_cast<p_shell_code_generator_type>(&ShellCodeGenerator<Func*, Args&&...>);
#endif

			p_shell_code_generator_type p_shellcode{};

			static size_t count{};
			static p_shell_code_generator_type orig_generator[MAX_FUNC_BUFFERED]{};
			static p_shell_code_generator_type alloc_generator[MAX_FUNC_BUFFERED]{};

			unsigned index{};
			while (orig_generator[index])
			{
				if (orig_generator[index] == self_addr)
				{
					p_shellcode = alloc_generator[index];
					break;
				}
				index++;
			}

			if (!p_shellcode)
			{
				p_shellcode = reinterpret_cast<p_shell_code_generator_type>(LocateShellCode(self_addr));
				orig_generator[count] = self_addr;
				alloc_generator[count] = p_shellcode;
				count++;
			}

			return p_shellcode(funcPtr, args...);
		}
	};
}
