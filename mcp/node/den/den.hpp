#pragma once

#include <libdevcore/Address.h>
#include <libdevcore/Common.h>
#include <mcp/common/common.hpp>
#include <unordered_map>
#include <vector>
#include <mcp/core/log_entry.hpp>

namespace mcp
{
    struct mining_ping
    {
        uint64_t mci;
        block_hash hash;
        uint32_t time;
        bool receive;
    };

    struct reward_a_day
    {
        dev::u256 all_reward;
        dev::u256 frozen_reward;
    };
    
    struct unit
    {
        dev::Address addr;
        uint32_t stake_factor;  //rang [0,10000]
        uint32_t last_calc_day;
        dev::u256 cur_rewords;
        std::map<uint32_t, reward_a_day> frozen;
        bool last_receive;  //true: last ping received. false: last ping not received.
        uint32_t online_score;  //rang [0,10000]
        std::map<uint32_t, std::map<uint8_t, mining_ping>> pings;
    };

    struct den_param
    {
        dev::u256 max_stake;
        dev::u256 max_reward_perday;
    };

    class den
    {
    public:
        den(){}
        void handle_den_mining_event(const log_entries &log_a);
        bool calculate_rewards(const dev::Address &addr, const uint32_t time, dev::u256 &give_rewards, dev::u256 &frozen_rewards, bool provide);
        void set_cur_time(const uint32_t &time);
    
    private:
        void set_max_stake(const dev::u256 &v);
        void set_max_reward_perday(const dev::u256 &v);
        void witelist_add(const dev::Address &addr);
        void witelist_remove(const dev::Address &addr);
        void stake(const dev::Address &addr, dev::u256 &v);
        void unstake(const dev::Address &addr, dev::u256 &v);

        den_param m_param;
        std::unordered_map<dev::Address, unit> m_dens;
    };
}