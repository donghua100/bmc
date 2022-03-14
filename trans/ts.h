#include<smt-switch/smt.h>
#include<unordered_map>
#include<string>


class TransitionSystem{

    public:
    smt::Term init_;
    smt::Term trans_;
    smt::UnorderedTermSet statevars_;
    smt::UnorderedTermSet next_statevars_;
    smt::UnorderedTermMap inputvars_;
    std::unordered_map<std::string,smt::Term> named_terms_;
    std::unordered_map<smt::Term,std::string> term_to_name_;
    smt::UnorderedTermMap state_updates_;
    smt::UnorderedTermMap next_map_;
    smt::UnorderedTermMap curr_map_;


    void name_term(const std::string name, const smt::Term & t);
    smt::Term make_inputvar(const std::string name, const smt::Sort & sort);
    smt::Term make_statevar(const std::string name, const smt::Sort & sort);
    void assign_next(const smt::Term & state, const smt::Term & val);
    bool is_input_var(const smt::Term & t) const;
    void promote_inputvar(const smt::Term & iv);
    void add_constraint(const smt::Term & constraint,
                    bool to_init_and_next = true);
    void constrain_init(const smt::Term & constraint);

};