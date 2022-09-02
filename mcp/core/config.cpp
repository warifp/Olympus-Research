#include "config.hpp"
#include <mcp/core/block_cache.hpp>
#include <mcp/core/block_store.hpp>
#include <mcp/db/db_transaction.hpp>
#include <mcp/common/log.hpp>

mcp::mcp_networks mcp::mcp_network = mcp::mcp_networks::mcp_live_network;
dev::u256 mcp::gas_price;
uint64_t mcp::chain_id;
mcp::log config_log = { mcp::log("node") };

bool mcp::is_test_network()
{
	return mcp_network == mcp_networks::mcp_test_network || mcp_network == mcp_networks::mcp_mini_test_network;
}

mcp::uint256_t mcp::chainID()
{
	return mcp::chain_id;
}

std::map<uint64_t, mcp::block_param> mcp::param::block_param_map = {};
std::map<uint64_t, mcp::witness_param> mcp::param::witness_param_map = {};
dev::SharedMutex mcp::param::m_mutex_witness;

const std::vector<std::string> mini_test_witness_str_list_v0 = {
	"0x1144B522F45265C2DFDBAEE8E324719E63A1694C"
};
const std::vector<std::string> test_witness_str_list_v0 = {
	"0x49a1b41e8ccb704f5c069ef89b08cd33f764e9b3",
	"0xf0821dc4ba9419b865aa412170377ca3b44cdb58",
	"0x329e6b5b8e59fc73d892958b2ff6a89474e3d067",
	"0x827cce78dc6ec7051f2d7bb9e7adaefba7ca3248",
	"0x918d3fe1dbff02fc7521d4a04b50017ce1a7c2ea",
	"0x929f336edb0a39ad5532a462d4a84e1546c5e5de",
	"0x1895ac1edc15389b905bb19537eb0c5b33d8c77a",
	"0x05174fa7ab39a36391b17850a2db9afdcf57190e",
	"0xa11b98c54d4189adda8eda97e13c214fedaf0a0f",
	"0xa65ec5c65031d668094cb1b81bb8253ea64a23d7",
	"0xba618c1e3e90d16e6c15d92ed198780dc4ad39c2",
	"0xc2cf7b9eb048c34c2b00175a884543366bbcd029",
	"0xc543a3868f3613eecd109761f71e31832ecf51ba",
	"0xdab8a5fb82eb24ad321751bb2dd8e4cc9a4e45e5"
};
const std::vector<std::string> beta_witness_str_list_v0 = {
	"0x6d76b7de9fa746bdfe2d5462ff46778a06bb2c35",
	"0x7f4f900abde901c79c1fe91a81ccd876595eceac",
	"0x94ab8f03fffc515d332894ea4be45df8aeacff4e",
	"0x545c6ddf180635303a27d92954da916dde931006",
	"0xa5356ce9415722e6c71a66c31cea172c2ccd7d90",
	"0xac8720f7149e200b479cf0325d7d36e491c410c4",
	"0xae8b58cc95649df86ed4583c57d136ee6c057f74",
	"0xb3cb7476c6241a6a72809727ebe0cf2db5bec98d",
	"0xb5bb1e0e692d8e7cfd2b17d220318dded1f34eb4",
	"0xb62e7871da077799a5c834565d8c162da3ee334e",
	"0xb75bfe4aa1e9aa99a1d87017d68d023e2cca48ae",
	"0xc757c14c4e20d604227c27935cd9f37150d27626",
	"0xd4c19e0c6a219e3a0e0b7249667cea21a69a6fdc",
	"0xdf691895cf79f2ca139b3e5d0714280877971eea"
};
const std::vector<std::string> live_witness_str_list_v0 = {
	"0x1EBEB508001C6F8FC1F87114DAE750D340EB402F",
	"0x2E308F70360D93307AF7EF8360B6AB5C521855BF",
	"0x3EA5BB9580A34DD866B2C831A3A6C277392BC18C",
	"0x9CABCF9D976EFACF73D8D03ABDF1C04E9911F00A",
	"0x31BAD08FE6B8E595763970F7D4CC219DE447C98C",
	"0x88F76825F13A98D0BBB32B2AC70AAADD2ECE4B67",
	"0x299F85C02DB107FF870E7085FE4FDFEEAB23D745",
	"0x389E1CCDE77191F42FD935A29EFF787B76BD6C0E",
	"0x424CEF3F560CD5ECF8EBB5BEE2D85F266452C49C",
	"0x0485E42D1C146E6A2E5C902A739E8BCB0603C141",
	"0x713F1D0BA5CD198BE4C539B2EB6C9A450479451D",
	"0x89146AB369D9F8C3699C2B9B061CDBF312052528",
	"0x381170D03819F5F1EB41FF6FE5D403470D13DB92",
	"0x474360B9DA0ADD7E598D6711B0E89BC7A952FC6C"
};

mcp::witness_param mcp::param::find_by_last_epoch(mcp::block_store &store_a, std::shared_ptr<mcp::block_cache> cache_a, uint64_t const & epoch_a, std::map<uint64_t, mcp::witness_param> const & maps_a)
{
	for (auto it(maps_a.rbegin()); it != maps_a.rend(); it++)
	{
		uint64_t const & min_last_epoch(it->first);
		if (epoch_a >= min_last_epoch)
		{
			mcp::witness_param const & result(it->second);
			return result;
		}
	}

	LOG(config_log.info) << "[find_by_last_epoch] epoch=" << epoch_a << " search witness from db";
	mcp::witness_param param(maps_a.rbegin()->second);
	param.witness_list.clear();
	mcp::db::db_transaction transaction(store_a.create_transaction());
	if (epoch_a == 0)
	{
		switch (mcp::mcp_network)
		{
			case mcp::mcp_networks::mcp_mini_test_network:
			{
				param.witness_list = to_witness_list(mini_test_witness_str_list_v0);
				break;
			}
			case mcp::mcp_networks::mcp_test_network:
			{
				param.witness_list = to_witness_list(test_witness_str_list_v0);
				break;
			}
			case mcp::mcp_networks::mcp_beta_network:
			{
				param.witness_list = to_witness_list(beta_witness_str_list_v0);
				break;
			}
			case mcp::mcp_networks::mcp_live_network:
			{
				param.witness_list = to_witness_list(live_witness_str_list_v0);
				break;
			}
			default:
				assert_x_msg(false, "Invalid network");
		}
	}
	else
	{
		epoch_elected_list list;
		store_a.epoch_elected_approve_receipts_get(transaction, epoch_a, list);
		for (auto hash : list.hashs)
		{
			auto approve_receipt = cache_a->approve_receipt_get(transaction, hash);
			if (approve_receipt) {
				param.witness_list.emplace(approve_receipt->from());
			}
			else {
				assert_x(false);
			}
		}
	}
	assert_x(param.witness_list.size() == param.witness_count);
	return param;
}
