#pragma once

#include <com/strcat.hpp>
#include <com/variant.hpp>
#include <cstring>

namespace miu::cmd {

class message {
  public:
    auto payload_size() const { return sizeof(com::variant) * _args_count + _cmd_length; }
    auto size() const { return sizeof(message) + payload_size(); }

    auto args_count() const { return _args_count; }
    com::variant const* args() const { return _args; }

    auto cmd_length() const { return _cmd_length; }
    auto cmd() const { return (const char*)(args() + _args_count); }

    auto seq_num() const { return _seq_num; }

  public:
    template<typename... ARGS>
    static message const*
    create(char* buf, uint32_t len, uint32_t seq_num, com::strcat const& strcat, ARGS&&... args) {
        return do_create(buf, len, seq_num, strcat.str(), std::vector<com::variant> { args... });
    }

    static message const* do_create(char* buf,
                                    uint32_t len,
                                    uint32_t seq_num,
                                    std::string_view cmd,
                                    std::vector<com::variant> const& args) {
        auto args_count = args.size();
        auto args_size  = sizeof(com::variant) * args_count;
        auto cmd_length = cmd.size() + 1;
        auto size       = sizeof(message) + args_size + cmd_length;
        message* msg    = nullptr;
        if (size <= len) {
            msg = new (buf) message;

            msg->_args_count = args_count;
            msg->_cmd_length = cmd_length;
            msg->_seq_num    = seq_num;
            std::memcpy(msg->_args, args.data(), args_size);
            std::strncpy((char*)(msg->args() + msg->args_count()), cmd.data(), cmd_length);
        }
        return msg;
    }

  private:
    uint16_t _args_count;
    uint16_t _cmd_length;
    uint32_t _seq_num;
    uint32_t _padding[2];
    com::variant _args[];
};
static_assert(sizeof(message) == sizeof(com::variant));

}    // namespace miu::cmd
