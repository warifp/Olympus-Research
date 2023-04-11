#pragma once

#include <libdevcore/Address.h>
#include <libdevcore/Common.h>
#include <mcp/common/common.hpp>
#include <unordered_map>
#include <vector>
#include <mcp/core/log_entry.hpp>
#include <mcp/core/timeout_db_transaction.hpp>
#include <mcp/common/log.hpp>

namespace mcp
{
    const uint32_t den_reward_period = 11; //default 3600 second = 1 hour
    struct den_ping
    {
        //uint64_t mci;
        //block_hash hash;
        uint64_t time;
        bool receive = false;
    };

    struct den_reward_a_day
    {
        dev::u256 all_reward;
        dev::u256 frozen_reward;
    };
    
    class den_unit
    {
    public:
        den_unit(){}
        dev::Address addr;
        uint64_t init_time;
        uint32_t stake_factor = 10000;  //rang [0,10000]
        dev::u256 rewards; //storage
        std::map<uint64_t, dev::u256> frozen; //storage
        uint64_t last_calc_day = 0; //storage

        bool last_receive = true;  //true: last ping received. false: last ping not received. storage
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
        void handle_den_mining_event(const log_entries &log_a);
        void handle_den_mining_ping(mcp::db::db_transaction & transaction_a, const dev::Address &addr, const uint64_t &time, bool ping, std::map<uint64_t, std::map<uint8_t, den_ping>>& pings);
        bool calculate_rewards(const dev::Address &addr, const uint64_t time, dev::u256 &give_rewards, dev::u256 &frozen_rewards, bool provide);
        void set_cur_time(const uint64_t &time);
        void set_mc_block_time(const uint64_t &time, const block_hash &h);
        bool is_mining(const dev::Address &addr){ return m_den_units.find(addr) != m_den_units.end(); }
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

        den_param m_param;
        std::unordered_map<dev::Address, den_unit> m_den_units;
        
		mcp::block_store & m_store;
        mcp::log m_log = { mcp::log("node") };
    };
    
    extern std::shared_ptr<mcp::den> g_den;
}