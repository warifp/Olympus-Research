#pragma once

#include <libdevcore/Address.h>
#include <libdevcore/Common.h>
#include <mcp/common/common.hpp>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <mcp/core/log_entry.hpp>
#include <mcp/core/timeout_db_transaction.hpp>
#include <mcp/common/log.hpp>
#include "account/abi.hpp"

namespace mcp
{
    const uint32_t den_reward_period = 11; //default 3600 second = 1 hour
    const uint32_t den_reward_period_day = den_reward_period * 24;
    struct den_ping
    {
        //uint64_t mci;
        //block_hash hash;
        uint64_t time;
        bool receive = false;
    };

    struct den_reward_a_day
    {
        dev::u256 release_a_day;
        dev::u256 frozen_reward;
    };

    enum den_event_type{
        EVENT_SET_PARAM = 0,
        EVENT_ADD_MINER,
        EVENT_DELETE_MINER,
        EVENT_STAKE,
        EVENT_UNSTAKE,
        EVENT_UNKNOWN
    };
    
    class den_unit
    {
    public:
        den_unit(){}
        den_unit(uint64_t init_time)
        {
            last_handle_ping_time = init_time;
            last_calc_day = init_time / den_reward_period_day;
        }
        //dev::Address addr;
        dev::u256 stakeAmount = 0;
        dev::u256 maxStake = 0;
        dev::u256 rewards = 0; //storage
        std::map<uint64_t, den_reward_a_day> frozen; //storage <day, den_reward_a_day>
        uint64_t last_calc_day; //storage

        bool last_receive = false;  //true: last ping received. false: last ping not received. storage
        uint64_t last_handle_ping_time = 0; //storage
        uint32_t no_ping_times = 0; //storage
        uint32_t ping_lose_time = 0; //storage
        uint32_t online_score = 10000;  //rang [0,10000]  storage

        void rewards_get(dev::RLP const & rlp);
		void rewards_streamRLP(RLPStream& s);

    private:
        mcp::log m_log = { mcp::log("node") };
    };

    struct den_param
    {
        dev::u256 max_stake;
        dev::u256 max_reward_perday = 1000000000000;
    };

    class block_store;
    class den
    {
    public:
        den(mcp::block_store& store_a);
        void handle_den_mining_event(mcp::db::db_transaction & transaction_a, const log_entries &log_a, const uint64_t &time);
        void handle_den_mining_ping(mcp::db::db_transaction & transaction_a, const dev::Address &addr, const uint64_t &time, bool ping, std::map<uint64_t, std::map<uint8_t, den_ping>>& pings);
        bool calculate_rewards(const dev::Address &addr, const uint64_t time, dev::u256 &give_rewards, dev::u256 &frozen_rewards);
        void set_cur_time(const uint64_t &time);
        void set_mc_block_time(const uint64_t &time, const block_hash &h);
        bool is_miner(const dev::Address &addr){ return m_den_units.find(addr) != m_den_units.end(); }
        uint64_t last_handle_ping_time(const dev::Address &addr);
        static bool need_ping(const dev::Address &addr, const block_hash &h);
        void init(mcp::db::db_transaction & transaction_a);
    
    private:
        void set_max_stake(const dev::u256 &v);
        void set_max_reward_perday(const dev::u256 &v);
        void witelist_add(const dev::Address &addr);
        void witelist_remove(const dev::Address &addr);
        void stake(const dev::Address &addr, dev::u256 &v);
        void unstake(const dev::Address &addr, dev::u256 &v);

        void handle_event_setparam(mcp::db::db_transaction & transaction_a, const log_entry &log_a, const uint64_t &time);
        void handle_event_addminer(mcp::db::db_transaction & transaction_a, const log_entry &log_a, const uint64_t &time);
        void handle_event_deleteminer(mcp::db::db_transaction & transaction_a, const log_entry &log_a);
        void handle_event_stake(mcp::db::db_transaction & transaction_a, const log_entry &log_a, const uint64_t &time);
        void handle_event_unstake(mcp::db::db_transaction & transaction_a, const log_entry &log_a, const uint64_t &time);
        den_event_type get_event_type(const std::string& eventName);

        den_param m_param;
        std::unordered_map<dev::Address, den_unit> m_den_units;
        std::unordered_set<dev::Address> m_den_witelist;
        dev::ABI m_abi;
		mcp::block_store & m_store;
        mcp::log m_log = { mcp::log("node") };
    };
    
    extern std::shared_ptr<mcp::den> g_den;
}