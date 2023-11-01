// See the file "COPYING" in the main distribution directory for copyright.

#pragma once

#include <sys/types.h> // for u_char
#include <cctype>
#include <map>
#include <set>
#include <string>

#include "zeek/CCL.h"
#include "zeek/EquivClass.h"
#include "zeek/List.h"

using cce_func = int (*)(int);

// This method is automatically generated by flex and shouldn't be namespaced
extern int re_lex(void);

namespace zeek {

class String;
class RE_Matcher;

namespace detail {

class NFA_Machine;
class DFA_Machine;
class DFA_State;
class Specific_RE_Matcher;
class CCL;

extern bool case_insensitive;
extern bool re_single_line;
extern CCL* curr_ccl;
extern NFA_Machine* nfa;
extern Specific_RE_Matcher* rem;
extern const char* RE_parse_input;

extern int clower(int);
extern void synerr(const char str[]);

using AcceptIdx = int;
using AcceptingSet = std::set<AcceptIdx>;
using MatchPos = uint64_t;
using AcceptingMatchSet = std::map<AcceptIdx, MatchPos>;
using string_list = name_list;

enum match_type { MATCH_ANYWHERE, MATCH_EXACTLY };

// A "specific" RE matcher will match one type of pattern: either
// MATCH_ANYWHERE or MATCH_EXACTLY.

class Specific_RE_Matcher {
public:
    explicit Specific_RE_Matcher(match_type mt, bool multiline = false);
    ~Specific_RE_Matcher();

    void AddPat(const char* pat);

    void MakeCaseInsensitive();
    void MakeSingleLine();

    void SetPat(const char* pat) { pattern_text = pat; }

    bool Compile(bool lazy = false);

    // The following is vestigial from flex's use of "{name}" definitions.
    // It's here because at some point we may want to support such
    // functionality.
    std::string LookupDef(const std::string& def);

    void InsertCCL(const char* txt, CCL* ccl) { ccl_dict[std::string(txt)] = ccl; }
    int InsertCCL(CCL* ccl) {
        ccl_list.push_back(ccl);
        return ccl_list.length() - 1;
    }
    CCL* LookupCCL(const char* txt) {
        const auto& iter = ccl_dict.find(std::string(txt));
        if ( iter != ccl_dict.end() )
            return iter->second;

        return nullptr;
    }
    CCL* LookupCCL(int index) { return ccl_list[index]; }
    CCL* AnyCCL(bool single_line_mode = false);

    void ConvertCCLs();

    bool MatchAll(const char* s);
    bool MatchAll(const String* s);

    // Compiles a set of regular expressions simultaneously.
    // 'idx' contains indices associated with the expressions.
    // On matching, the set of indices is returned which correspond
    // to the matching expressions.  (idx must not contain zeros).
    bool CompileSet(const string_list& set, const int_list& idx);

    // For use with CompileSet() to collect indices of all matched
    // expressions into the matches vector. The matches vector is
    // populated with the indices of all matching expressions provided
    // to CompileSet()'s set and idx arguments.
    //
    // Behaves as MatchAll(), consuming the complete input string.
    bool MatchSet(const String* s, std::vector<AcceptIdx>& matches);

    // Returns the position in s just beyond where the first match
    // occurs, or 0 if there is no such position in s.  Note that
    // if the pattern matches empty strings, matching continues
    // in an attempt to match at least one character.
    int Match(const char* s);
    int Match(const String* s);
    int Match(const u_char* bv, int n);

    int LongestMatch(const char* s);
    int LongestMatch(const String* s);
    int LongestMatch(const u_char* bv, int n, bool bol = true, bool eol = true);

    EquivClass* EC() { return &equiv_class; }

    const char* PatternText() const { return pattern_text.c_str(); }

    DFA_Machine* DFA() const { return dfa; }

    void Dump(FILE* f);

protected:
    void AddAnywherePat(const char* pat);
    void AddExactPat(const char* pat);

    // Used by the above.  orig_fmt is the format to use when building
    // up a new pattern_text from the given pattern; app_fmt is for when
    // appending to an existing pattern_text.
    void AddPat(const char* pat, const char* orig_fmt, const char* app_fmt);

    bool MatchAll(const u_char* bv, int n, std::vector<AcceptIdx>* matches = nullptr);

    match_type mt;
    bool multiline;

    std::string pattern_text;

    std::map<std::string, std::string> defs;
    std::map<std::string, CCL*> ccl_dict;
    std::vector<char> modifiers;
    PList<CCL> ccl_list;
    EquivClass equiv_class;
    int* ecs;
    DFA_Machine* dfa;
    AcceptingSet* accepted;

    CCL* any_ccl;
    CCL* single_line_ccl;
};

class RE_Match_State {
public:
    explicit RE_Match_State(Specific_RE_Matcher* matcher) {
        dfa = matcher->DFA() ? matcher->DFA() : nullptr;
        ecs = matcher->EC()->EquivClasses();
        current_pos = -1;
        current_state = nullptr;
    }

    const AcceptingMatchSet& AcceptedMatches() const { return accepted_matches; }

    // Returns the number of bytes fed into the matcher so far
    int Length() { return current_pos; }

    // Returns true if this inputs leads to at least one new match.
    // If clear is true, starts matching over.
    bool Match(const u_char* bv, int n, bool bol, bool eol, bool clear);

    void Clear() {
        current_pos = -1;
        current_state = nullptr;
        accepted_matches.clear();
    }

    void AddMatches(const AcceptingSet& as, MatchPos position);

protected:
    DFA_Machine* dfa;
    int* ecs;

    AcceptingMatchSet accepted_matches;
    DFA_State* current_state;
    int current_pos;
};

extern RE_Matcher* RE_Matcher_conjunction(const RE_Matcher* re1, const RE_Matcher* re2);
extern RE_Matcher* RE_Matcher_disjunction(const RE_Matcher* re1, const RE_Matcher* re2);

} // namespace detail

class RE_Matcher final {
public:
    RE_Matcher();
    explicit RE_Matcher(const char* pat);
    RE_Matcher(const char* exact_pat, const char* anywhere_pat);
    ~RE_Matcher();

    void AddPat(const char* pat);

    // Makes the matcher as specified to date case-insensitive.
    void MakeCaseInsensitive();
    bool IsCaseInsensitive() const { return is_case_insensitive; }

    void MakeSingleLine();
    bool IsSingleLine() const { return is_single_line; }

    bool Compile(bool lazy = false);

    // Returns true if s exactly matches the pattern, false otherwise.
    bool MatchExactly(const char* s) { return re_exact->MatchAll(s); }
    bool MatchExactly(const String* s) { return re_exact->MatchAll(s); }

    // Returns the position in s just beyond where the first match
    // occurs, or 0 if there is no such position in s.  Note that
    // if the pattern matches empty strings, matching continues
    // in an attempt to match at least one character.
    int MatchAnywhere(const char* s) { return re_anywhere->Match(s); }
    int MatchAnywhere(const String* s) { return re_anywhere->Match(s); }

    // Note: it matches the *longest* prefix and returns the
    // length of matched prefix. It returns -1 on mismatch.
    int MatchPrefix(const char* s) { return re_exact->LongestMatch(s); }
    int MatchPrefix(const String* s) { return re_exact->LongestMatch(s); }
    int MatchPrefix(const u_char* s, int n) { return re_exact->LongestMatch(s, n); }

    // MatchPrefix() version allowing control of bol and eol.
    // This can be useful when searching for a pattern with an
    // anchor within a larger string.
    int MatchPrefix(const u_char* s, int n, bool bol, bool eol) { return re_exact->LongestMatch(s, n, bol, eol); }

    bool Match(const u_char* s, int n) { return re_anywhere->Match(s, n); }

    const char* PatternText() const { return re_exact->PatternText(); }
    const char* AnywherePatternText() const { return re_anywhere->PatternText(); }

    // Original text used to construct this matcher.  Empty unless
    // the main ("explicit") constructor was used.
    const char* OrigText() const { return orig_text.c_str(); }

protected:
    std::string orig_text;

    detail::Specific_RE_Matcher* re_anywhere;
    detail::Specific_RE_Matcher* re_exact;

    bool is_case_insensitive = false;
    bool is_single_line = false;
};

} // namespace zeek
