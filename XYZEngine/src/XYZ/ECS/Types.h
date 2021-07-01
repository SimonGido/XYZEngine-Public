#pragma once
#include <bitset>

namespace XYZ {
	
	template <int I, class... Ts>
	decltype(auto) get(Ts&&... ts)
	{
		return std::get<I>(std::forward_as_tuple(ts...));
	}
	template<typename... Types>
	constexpr auto GetTypesSize()
	{
		return std::array<std::size_t, sizeof...(Types)>{sizeof(Types)...};
	}

	template<class F, class...Ts, std::size_t...Is>
	void ForEachInTuple(const std::tuple<Ts...>& tuple, F func, std::index_sequence<Is...>)
	{
		using expander = int[];
		(void)expander {
			0, ((void)func(std::get<Is>(tuple)), 0)...
		};
	}

	template<class F, class...Ts>
	void ForEachInTuple(const std::tuple<Ts...>& tuple, F func)
	{
		ForEachInTuple(tuple, func, std::make_index_sequence<sizeof...(Ts)>());
	}
	
}