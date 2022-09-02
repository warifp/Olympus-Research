#pragma once

#include <chrono>
#include <cstddef>
#include <set>
#include <map>
#include <utility>
#include <mcp/common/numbers.hpp>
#include <libdevcore/Address.h>
#include <libdevcore/Guards.h>

extern const std::vector<std::string> mini_test_witness_str_list_v0;
extern const std::vector<std::string> test_witness_str_list_v0;
extern const std::vector<std::string> beta_witness_str_list_v0;
extern const std::vector<std::string> live_witness_str_list_v0;

namespace mcp
{
using namespace dev;
// Network variants with different genesis blocks and network parameters
enum class mcp_networks
{
	mcp_live_network = 1,
	mcp_beta_network = 2,
	mcp_test_network = 3,
	mcp_mini_test_network = 4
};

extern mcp::mcp_networks mcp_network;

extern dev::u256 gas_price;
extern uint64_t chain_id;

bool is_test_network();
mcp::uint256_t chainID();

size_t const static max_data_size(32000); //32k
size_t const static skiplist_divisor(10);
uint64_t const static block_max_gas(8000000);
uint64_t const static max_link_block_size(2048);

class block_param
{
public:
	size_t max_parent_size;
	size_t max_link_size;
};

class witness_param
{
public:
	size_t witness_count;
	size_t majority_of_witnesses;
	std::set<dev::Address> witness_list;
};

class block_cache;
class block_store;
class param
{
public:
	static void init()
	{
		init_block_param();
		init_witness_param();
	}

	static mcp::block_param const & block_param(uint64_t const & last_summary_mci_a)
	{
		mcp::block_param const & b_param
			= find_by_last_summary_mci<mcp::block_param>(last_summary_mci_a, block_param_map);
		return b_param;
	}

	static mcp::witness_param get_witness_param(mcp::block_store &store_a, std::shared_ptr<mcp::block_cache> cache_a, uint64_t const & epoch_a)
	{
		DEV_READ_GUARDED(m_mutex_witness){
			mcp::witness_param const & w_param
				= find_by_last_epoch(store_a, cache_a, epoch_a, witness_param_map);
			return w_param;
		}
	}

	static bool is_witness(mcp::block_store &store_a, std::shared_ptr<mcp::block_cache> cache_a, uint64_t const & epoch_a, dev::Address const & account_a)
	{
		DEV_READ_GUARDED(m_mutex_witness){
			mcp::witness_param const & w_param = get_witness_param(store_a, cache_a, epoch_a);
			if (w_param.witness_list.count(account_a))
				return true;
			return false;
		}
	}

	static std::set<dev::Address> to_witness_list(std::vector<std::string> const & witness_strs)
	{
		std::set<dev::Address> witness_list;
		for (std::string w_str : witness_strs)
		{
			dev::Address w_acc(w_str);
			witness_list.insert(w_acc);
		}
		return witness_list;
	}

	static void add_witness_param(uint64_t const & epoch_a, mcp::witness_param &w_param){
		DEV_WRITE_GUARDED(m_mutex_witness){
			mcp::param::witness_param_map.insert({epoch_a, w_param });

			//Avoid continuous size growth
			if(mcp::param::witness_param_map.size() > 5){
				for(auto it=mcp::param::witness_param_map.begin(); it!=mcp::param::witness_param_map.end();){
					if(it->first <= epoch_a - 5){
						it = mcp::param::witness_param_map.erase(it);
					}
					else{
						return;
					}
				}
			}
		}
	}

	static uint64_t witness_param_size(){
		DEV_READ_GUARDED(m_mutex_witness){
			return witness_param_map.size();
		}
	}

private:
	static void init_block_param()
	{
		mcp::block_param b_param_v0;
		b_param_v0.max_parent_size = 16;
		b_param_v0.max_link_size = 4096;
		block_param_map.insert({ 0, b_param_v0 });

		//chain_id = (uint64_t)mcp::mcp_network;
		chain_id = (uint64_t)971;
		switch (mcp::mcp_network)
		{
		case mcp::mcp_networks::mcp_mini_test_network:
		{
			gas_price = 10000000;
			break;
		}
		case mcp::mcp_networks::mcp_test_network:
		{
			gas_price = 10000000;
			break;
		}
		case mcp::mcp_networks::mcp_beta_network:
		{
			gas_price = 10000000;
			break;
		}
		case mcp::mcp_networks::mcp_live_network:
		{
			gas_price = (uint256_t)5e13;
			break;
		}
		default:
			assert_x_msg(false, "Invalid network");
		}
	}

	static void init_witness_param()
	{

		switch (mcp::mcp_network)
		{
		case mcp::mcp_networks::mcp_mini_test_network:
		{
			mcp::witness_param w_param_v0;
			w_param_v0.witness_count = 1;
			w_param_v0.majority_of_witnesses = w_param_v0.witness_count * 2 / 3 + 1;
			w_param_v0.witness_list = to_witness_list(mini_test_witness_str_list_v0);
			assert_x(w_param_v0.witness_list.size() == w_param_v0.witness_count);

			witness_param_map.insert({ 0, w_param_v0 });
			break;
		}
		case mcp::mcp_networks::mcp_test_network:
		{
			mcp::witness_param w_param_v0;
			w_param_v0.witness_count = 14;
			w_param_v0.majority_of_witnesses = w_param_v0.witness_count * 2 / 3 + 1;
			w_param_v0.witness_list = to_witness_list(test_witness_str_list_v0);
			assert_x(w_param_v0.witness_list.size() == w_param_v0.witness_count);

			witness_param_map.insert({ 0, w_param_v0 });
			break;
		}
		case mcp::mcp_networks::mcp_beta_network:
		{
			mcp::witness_param w_param_v0;
			w_param_v0.witness_count = 14;
			w_param_v0.majority_of_witnesses = w_param_v0.witness_count * 2 / 3 + 1;
			w_param_v0.witness_list = to_witness_list(beta_witness_str_list_v0);
			assert_x(w_param_v0.witness_list.size() == w_param_v0.witness_count);

			witness_param_map.insert({ 0, w_param_v0 });
			break;
		}
		case mcp::mcp_networks::mcp_live_network:
		{
			mcp::witness_param w_param_v0;
			w_param_v0.witness_count = 14;
			w_param_v0.majority_of_witnesses = w_param_v0.witness_count * 2 / 3 + 1;
			w_param_v0.witness_list = to_witness_list(live_witness_str_list_v0);
			assert_x(w_param_v0.witness_list.size() == w_param_v0.witness_count);

			witness_param_map.insert({ 0, w_param_v0 });
			break;
		}
		default:
			assert_x_msg(false, "Invalid network");
		}
	}

	template<class T>
	static T const & find_by_last_summary_mci(uint64_t const & last_summary_mci_a, std::map<uint64_t, T> const & maps_a)
	{
		for (auto it(maps_a.rbegin()); it != maps_a.rend(); it++)
		{
			uint64_t const & min_last_summary_mci(it->first);
			if (last_summary_mci_a >= min_last_summary_mci)
			{
				T const & result(it->second);
				return result;
			}
		}
		assert_x(false);
	}

	static mcp::witness_param find_by_last_epoch(mcp::block_store &store_a, std::shared_ptr<mcp::block_cache> cache_a, uint64_t const & epoch_a, std::map<uint64_t, mcp::witness_param> const & maps_a);

	//min last summary mci -> block param
	static std::map<uint64_t, mcp::block_param> block_param_map;

	//min last summary mci -> witness param
	static std::map<uint64_t, mcp::witness_param> witness_param_map;
	static dev::SharedMutex m_mutex_witness;
};

}
