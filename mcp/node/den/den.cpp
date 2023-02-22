#include "den.hpp"
#include <algorithm>


void mcp::den::set_max_stake(const dev::u256 &v)
{
    m_param.max_stake = v;
}

void mcp::den::set_max_reward_perday(const dev::u256 &v)
{
    m_param.max_reward_perday = v;
}

void mcp::den::witelist_add(const dev::Address &addr)
{

}

void mcp::den::witelist_remove(const dev::Address &addr)
{

}

void mcp::den::stake(const dev::Address &addr, dev::u256 &v)
{

}

void mcp::den::unstake(const dev::Address &addr, dev::u256 &v)
{

}

void mcp::den::set_cur_time(const uint32_t &time)
{
    m_cur_time = time;
}

void mcp::den::handle_den_mining_event(const log_entries &log_a)
{

}

bool mcp::den::calculate_rewards(const dev::Address &addr, dev::u256 &give_rewards, dev::u256 &frozen_rewards)
{
    if(m_dens.count(addr)){
        auto &u = m_dens[addr];
        if(m_cur_day <= u.last_calc_day){
            return false;
        }
        static uint32_t ping_lose_time = 0;
        dev::u256 all_reward = 0;
        dev::u256 full_reward = m_param.max_reward_perday * u.stake_factor / 10000;
        for(std::map<uint32_t, std::vector<mining_ping>>::iterator it = u.pings.begin(); it != u.pings.end() && it->first < m_cur_day; it++){
            for(auto p : it->second){
                if(p.receive){
                    ping_lose_time = 0;
                    if(u.online_score < 10000){
                        u.online_score = std::min((uint32_t)10000, u.online_score+10000/168);
                        all_reward += full_reward * u.online_score / 10000;
                    }
                    else{
                        all_reward += full_reward;
                    }
                }
                else{
                    ping_lose_time += 1;
                    if(ping_lose_time >=2){
                        u.online_score = 0;
                    }
                }
            }
            
            if(u.frozen.find(it->first) == u.frozen.end()){
                u.frozen[it->first] = reward_a_day{all_reward, 0};
            }
            else{
                u.frozen[it->first].all_reward += all_reward;
            }
            u.frozen[it->first].frozen_reward += all_reward * 75 / 100;
            give_rewards += all_reward * 25 / 100;
            all_reward = 0;
        }

        for(std::map<uint32_t, reward_a_day>::iterator it = u.frozen.begin(); it != u.frozen.end() && it->first < m_cur_day;){
            if(m_cur_day - it->first >= 270){
                give_rewards += it->second.frozen_reward;
                u.frozen.erase(it++);
                continue;
            }
            auto give = it->second.all_reward * (m_cur_day-std::max(u.last_calc_day, it->first))/270;
            it->second.frozen_reward -= give;
            give_rewards += give;
            frozen_rewards += it->second.frozen_reward;
            it++;
        }
    }
    else{
        return false;
    }
}
