#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <limits>
#include <set>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

// Suffix automaton for arbitrary byte strings, including embedded NUL bytes.
//
// Transitions are stored in each state as a sorted vector of (byte, state)
// pairs. Lookup uses binary search; insertion shifts at most 255 entries.
// Because the alphabet has exactly 256 possible byte values, each state's
// transition vector has at most 256 entries. For n >= 1 the automaton has at
// most 2n states; for n == 0 it has only the root. Thus states plus
// transitions use O(n) space for nonempty byte strings (and O(1) for empty),
// with a potentially significant constant factor. More explicitly, for
// n >= 1 there are at most 2n states and at most 512n transition records.
//
// Construction is deterministic O(n) for a fixed byte alphabet: suffix-link
// traversal is amortized O(n), and each transition search, insertion, or
// clone copy costs at most O(256), a constant. Membership for a query of m
// bytes takes O(m log 256) = O(m) time, worst case; counting distinct
// substrings takes O(n) time. The distinct count is returned as uint64_t and
// throws std::overflow_error if the exact count cannot be represented.
class SuffixAutomaton {
public:
    explicit SuffixAutomaton(std::string_view text) {
        states_.push_back(State{0, no_state(), {}});
        for (char byte : text) {
            extend(static_cast<unsigned char>(byte));
        }
    }

    [[nodiscard]] bool contains(std::string_view pattern) const {
        std::size_t state = 0;
        for (char byte : pattern) {
            const std::size_t next = transition(
                state, static_cast<unsigned char>(byte));
            if (next == no_state()) {
                return false;
            }
            state = next;
        }
        return true;
    }

    [[nodiscard]] std::uint64_t distinct_nonempty_substrings() const {
        std::uint64_t total = 0;
        for (std::size_t i = 1; i < states_.size(); ++i) {
            const std::size_t parent = states_[i].link;
            const std::size_t contribution =
                states_[i].length - states_[parent].length;
            const std::uintmax_t add =
                static_cast<std::uintmax_t>(contribution);
            const std::uintmax_t maximum =
                static_cast<std::uintmax_t>(
                    std::numeric_limits<std::uint64_t>::max());
            if (add > maximum - static_cast<std::uintmax_t>(total)) {
                throw std::overflow_error(
                    "distinct substring count exceeds uint64_t");
            }
            total += static_cast<std::uint64_t>(add);
        }
        return total;
    }

private:
    struct State {
        std::size_t length;
        std::size_t link;
        std::vector<std::pair<unsigned char, std::size_t>> transitions;
    };

    static constexpr std::size_t no_state() noexcept {
        return std::numeric_limits<std::size_t>::max();
    }

    std::vector<State> states_;
    std::size_t last_ = 0;

    [[nodiscard]] std::size_t transition(std::size_t state,
                                         unsigned char byte) const {
        const auto& edges = states_[state].transitions;
        const auto it = std::lower_bound(
            edges.begin(), edges.end(), byte,
            [](const auto& edge, unsigned char value) {
                return edge.first < value;
            });
        if (it == edges.end() || it->first != byte) {
            return no_state();
        }
        return it->second;
    }

    void set_transition(std::size_t state, unsigned char byte,
                        std::size_t destination) {
        auto& edges = states_[state].transitions;
        const auto it = std::lower_bound(
            edges.begin(), edges.end(), byte,
            [](const auto& edge, unsigned char value) {
                return edge.first < value;
            });
        if (it != edges.end() && it->first == byte) {
            it->second = destination;
        } else {
            edges.insert(it, {byte, destination});
        }
    }

    void extend(unsigned char byte) {
        const std::size_t current = states_.size();
        states_.push_back(State{states_[last_].length + 1, no_state(), {}});

        std::size_t state = last_;
        while (state != no_state() && transition(state, byte) == no_state()) {
            set_transition(state, byte, current);
            state = states_[state].link;
        }

        if (state == no_state()) {
            states_[current].link = 0;
        } else {
            const std::size_t target = transition(state, byte);
            if (states_[state].length + 1 == states_[target].length) {
                states_[current].link = target;
            } else {
                const std::size_t clone = states_.size();
                states_.push_back(states_[target]);
                states_[clone].length = states_[state].length + 1;

                while (state != no_state() &&
                       transition(state, byte) == target) {
                    set_transition(state, byte, clone);
                    state = states_[state].link;
                }

                states_[target].link = clone;
                states_[current].link = clone;
            }
        }
        last_ = current;
    }
};

namespace {

void check(bool condition, const char* message) {
    if (!condition) {
        throw std::runtime_error(message);
    }
}

std::uint64_t brute_force_distinct_count(const std::string& text) {
    std::set<std::string> substrings;
    for (std::size_t begin = 0; begin < text.size(); ++begin) {
        for (std::size_t length = 1; begin + length <= text.size(); ++length) {
            substrings.insert(text.substr(begin, length));
        }
    }
    return static_cast<std::uint64_t>(substrings.size());
}

void test_named_cases() {
    const SuffixAutomaton empty("");
    check(empty.distinct_nonempty_substrings() == 0,
          "empty string distinct count");
    check(empty.contains(""), "empty pattern is present");
    check(!empty.contains("a"), "nonempty pattern absent from empty string");

    const SuffixAutomaton single("x");
    check(single.distinct_nonempty_substrings() == 1,
          "single-byte distinct count");
    check(single.contains("x"), "single byte membership");
    check(!single.contains("xx"), "too-long single-byte query");

    const SuffixAutomaton banana("banana");
    check(banana.distinct_nonempty_substrings() == 15,
          "banana distinct count");
    check(banana.contains("ana"), "banana contains ana");
    check(banana.contains("banana"), "banana contains itself");
    check(!banana.contains("band"), "banana does not contain band");

    const SuffixAutomaton repeated("zzzzzz");
    check(repeated.distinct_nonempty_substrings() == 6,
          "repeated-byte distinct count");
    check(repeated.contains("zzzz"), "repeated-byte membership");
    check(!repeated.contains("zzzzzzz"), "repeated-byte overlong query");

    const std::string nul_text("\0a\0", 3);
    const SuffixAutomaton with_nul(nul_text);
    check(with_nul.distinct_nonempty_substrings() == 5,
          "embedded-NUL distinct count");
    check(with_nul.contains(std::string_view(nul_text.data() + 1, 2)),
          "embedded-NUL suffix membership");
    check(with_nul.contains(std::string_view(nul_text.data(), 1)),
          "NUL-byte membership");
    const std::string absent_nul_pattern("\0\0", 2);
    check(!with_nul.contains(absent_nul_pattern),
          "absent embedded-NUL pattern");
}

void test_brute_force_short_strings() {
    constexpr char alphabet[] = {'a', 'b', '\0'};
    for (std::size_t length = 0; length <= 8; ++length) {
        std::size_t possibilities = 1;
        for (std::size_t i = 0; i < length; ++i) {
            possibilities *= 3;
        }
        for (std::size_t code = 0; code < possibilities; ++code) {
            std::size_t remaining = code;
            std::string text(length, '\0');
            for (std::size_t position = 0; position < length; ++position) {
                text[position] = alphabet[remaining % 3];
                remaining /= 3;
            }
            const SuffixAutomaton automaton(text);
            check(automaton.distinct_nonempty_substrings() ==
                      brute_force_distinct_count(text),
                  "deterministic brute-force distinct-count comparison");
        }
    }
}

}  // namespace

int main() {
    try {
        test_named_cases();
        test_brute_force_short_strings();
        std::cout << "All suffix automaton tests passed.\n";
    } catch (const std::exception& error) {
        std::cerr << "Suffix automaton test failure: " << error.what() << '\n';
        return 1;
    }
    return 0;
}
