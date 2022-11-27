/* ************************************************************************* *
 * AUTHOR:      Noah Krim
 * ASSIGNMENT:  Lab 3 - CPU Lab
 * CLASS:       UCD - ECS 154A
 * ------------------------------------------------------------------------- *
 * File: alarmas.cpp
 *  Asssembler that encodes a human-readable alARM program from a `.s` file
 *  into hex machine code that is runnable with an alARM Logisim CPU
 * ************************************************************************* */

#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <string>
#include <cstring>
#include <vector>
#include <array>
#include <map>
#include <set>
#include <cstdint>
#include <algorithm>
#include <regex>

using namespace std;


/* ========================================================================= *
 * Macros
 * ========================================================================= */
#define WIDTH_TO_BITS(X) ~((-1u)<<X) //((1u<<X)-1u) 


/* ========================================================================= *
 * Opcode and Bitmask Enums
 * ========================================================================= */
enum OPCODE : uint16_t {
    NOP  =0b0000000<<9,
    HALT =0b0000011<<9,
    MOVRR=0b0000100<<9,
    MOVRF=0b0000110<<9,
    MOVFR=0b0000111<<9,
    LDRO =0b0001000<<9,
    LDR  =0b0001011<<9,
    STRO =0b0001100<<9,
    STR  =0b0001111<<9,

    ADD  =0b0010000<<9,
    SUB  =0b0010001<<9,
    MUL  =0b0010010<<9,
    MULU =0b0010011<<9,
    DIV  =0b0010100<<9,
    MOD  =0b0010101<<9,
    AND  =0b0010110<<9,
    OR   =0b0010111<<9,
    EOR  =0b0011000<<9,
    NOT  =0b0011001<<9,
    LSL  =0b0011010<<9,
    LSR  =0b0011011<<9,
    ASR  =0b0011100<<9,
    ROL  =0b0011101<<9,
    ROR  =0b0011110<<9,
    CMP  =0b0011111<<9,

    B    =0b0100000<<9,
    BEQ  =0b0110000<<9,
    BNE  =0b0111000<<9,
    MOVIM=0b1000000<<9,
};

enum I_FMT {
    S_TYPE=0,
    R1_TYPE,
    R2_TYPE,
    R2NW_TYPE,
    R3_TYPE,
    B_TYPE,
    I_TYPE,
    FL_TYPE,
    FS_TYPE,
    LS_TYPE,
    LSO_TYPE,
    FMT_LEN
};

enum OPR_WIDTH {
    NON=0,
    REG=3,
    IMM=12
};


/* ========================================================================= *
 * Static Constant Definitions
 * ========================================================================= */
const unsigned WORD_SIZE = 16;
const long long IMM_MAX = (1<<(IMM-1))-1;
const long long IMM_MIN = -1ULL<<(IMM-1);
const unsigned IMM_NIBS = (IMM>>2) + !!(IMM&0b11);
const unsigned MAX_INST = 65536;
const unsigned MAX_OPR = 3;
const unsigned MAX_REG = WIDTH_TO_BITS(REG);
const char* ORD_SUFXS[] = { "st", "nd", "rd", "th" }; 


/* ========================================================================= *
 * Forward declare structs
 * ========================================================================= */
struct label_s;
struct prog_opts_s;
struct prog_s;

/* ========================================================================= *
 * Typedefs
 * ========================================================================= */
typedef uint16_t     mword_t;

typedef vector<string>          inst_tokens_t;
typedef map<string, mword_t>    label_map_t;

typedef vector<pair<uint8_t,OPR_WIDTH>>     fmt_config_t;

typedef vector<pair<OPCODE,inst_tokens_t>>      inst_list_t;


/* ========================================================================= *
 * Struct definitions
 * ========================================================================= */
struct label_s {
    string      name;
    mword_t     address;
};

struct prog_opts_s {
    char*   src_file    = nullptr;
    char*   out_file    = nullptr;
    bool    list_flag   = false;
    bool    strict_flag = false;
};

struct prog_s {
    inst_list_t         insts;
    label_map_t         label_lookup;
    vector<label_s>     labels;
    vector<unsigned>    debug_line_nums;
    vector<mword_t>     mcode;
    vector<inst_tokens_t> insts_raw;
};


/* ========================================================================= *
 * ISA Config Definition
 * ========================================================================= */
const array<fmt_config_t, FMT_LEN> FMT_CONFIG = {{
    { },                                    // S-Type
    { {1*REG,REG} },                        // R1-Type
    { {1*REG,REG}, {2*REG,REG} },           // R2-Type
    { {2*REG,REG}, {0,REG} },               // R2NW-Type
    { {1*REG,REG}, {2*REG,REG}, {0,REG} },  // R3-Type
    { {0,IMM} },                            // B-Type
    { {1*IMM,REG}, {0,IMM} },               // I-Type
    { {1*REG,REG}, {0,NON} },               // FL-Type
    { {0,NON},     {2*REG,REG} },           // FS-Type
    { {1*REG,REG}, {2*REG,REG} },           // LS-Type
    { {1*REG,REG}, {2*REG,REG}, {0,REG} },  // LSO-Type
}};

const map<OPCODE, I_FMT> OPC_TO_FMT = {
    { NOP,  S_TYPE },
    { HALT, S_TYPE },
    { MOVRR, R2_TYPE },
    { MOVIM, I_TYPE },
    { MOVRF, FL_TYPE },
    { MOVFR, FS_TYPE },
    { LDR,  LS_TYPE },
    { LDRO, LSO_TYPE },
    { STR,  LS_TYPE },
    { STRO, LSO_TYPE },
    { ADD,  R3_TYPE },
    { SUB,  R3_TYPE },
    { MUL,  R3_TYPE },
    { MULU, R3_TYPE },
    { DIV,  R3_TYPE },
    { MOD,  R3_TYPE },
    { AND,  R3_TYPE },
    { OR,   R3_TYPE },
    { EOR,  R3_TYPE },
    { NOT,  R2_TYPE },
    { LSL,  R3_TYPE },
    { LSR,  R3_TYPE },
    { ASR,  R3_TYPE },
    { ROL,  R3_TYPE },
    { ROR,  R3_TYPE },
    { CMP,  R2NW_TYPE },
    { B,    B_TYPE },
    { BEQ,  B_TYPE },
    { BNE,  B_TYPE },
};

const map<string, vector<OPCODE>> ISA = {
    {"NOP",     { NOP }}, 
    {"HALT",    { HALT }},
    {"MOV",     { MOVRR, MOVRF, MOVFR, MOVIM }},
    {"LDR",     { LDR, LDRO }},
    {"STR",     { STR, STRO }},  
    {"ADD",     { ADD }},
    {"SUB",     { SUB }},  
    {"MUL",     { MUL }},  
    {"MULU",    { MULU }}, 
    {"DIV",     { DIV }},  
    {"MOD",     { MOD }},  
    {"AND",     { AND }},  
    {"OR",      { OR }},   
    {"EOR",     { EOR }},  
    {"NOT",     { NOT }},  
    {"LSL",     { LSL }},  
    {"LSR",     { LSR }},  
    {"ASR",     { ASR }},  
    {"ROL",     { ROL }},  
    {"ROR",     { ROR }},  
    {"CMP",     { CMP }},  
    {"B",       { B }},    
    {"BEQ",     { BEQ }},    
    {"BNE",     { BNE }},
};

const array<regex,FMT_LEN> FMT_REGEX = {{
    // S_TYPE
    regex("^\\s*$", regex::icase),
    // R1_TYPE
    regex("^(R\\d+)\\s*$", regex::icase),  
    // R2_TYPE
    regex("^(R\\d+)(?:\\s+|\\s*,\\s*)(R\\d+)\\s*$", regex::icase), 
    // R2NW_TYPE
    regex("^(R\\d+)(?:\\s+|\\s*,\\s*)(R\\d+)\\s*$", regex::icase), 
    // R3_TYPE
    regex(string("^(R\\d+)(?:\\s+|\\s*,\\s*)")
        + string("(R\\d+)(?:\\s+|\\s*,\\s*)(R\\d+)\\s*$"), regex::icase), 
    // B_TYPE
    regex("^([-\\w]+)\\s*$", regex::icase), 
    // I_TYPE
    regex("^(R\\d+)(?:\\s+|\\s*,\\s*)([-\\w]+)\\s*$", regex::icase), 
    // FL_TYPE
    regex("^(R\\d+)(?:\\s+|\\s*,\\s*)(FLAGS)\\s*$", regex::icase),
    // FS_TYPE 
    regex("^(FLAGS)(?:\\s+|\\s*,\\s*)(R\\d+)\\s*$", regex::icase), 
    // LS_TYPE
    regex(string("^(R\\d+)(?:\\s*,\\s*\\[?|\\s*\\[|\\s)\\s*(R\\d+)")
        + string("\\s*\\]?\\s*$"), regex::icase),
    // LSO_TYPE
    regex(string("^(R\\d+)(?:\\s*,\\s*\\[?|\\s*\\[|\\s)\\s*(R\\d+)")
        + string("(?:\\s+|\\s*,\\s*)(R\\d+)\\s*\\]?\\s*$"), regex::icase)
}};

const array<regex,FMT_LEN> FMT_REGEX_STRICT = {{
    // S_TYPE
    regex("^$", regex::icase),
    // R1_TYPE
    regex("^(R\\d+)$", regex::icase),  
    // R2_TYPE
    regex("^(R\\d+)\\s*,\\s*(R\\d+)$", regex::icase), 
    // R2NW_TYPE
    regex("^(R\\d+)\\s*,\\s*(R\\d+)$", regex::icase), 
    // R3_TYPE
    regex("^(R\\d+)\\s*,\\s*(R\\d+)\\s*,\\s*(R\\d+)$", regex::icase), 
    // B_TYPE
    regex("^(\\w+)$", regex::icase), 
    // I_TYPE
    regex("^(R\\d+)\\s*,\\s*(\\w+)$", regex::icase), 
    // FL_TYPE
    regex("^(R\\d+)\\s*,\\s*(FLAGS)$", regex::icase),
    // FS_TYPE 
    regex("^(FLAGS)\\s*,\\s*(R\\d+)$", regex::icase), 
    // LS_TYPE
    regex("^(R\\d+)\\s*,\\s*\\[\\s*(R\\d+)\\s*\\]$", regex::icase), 
    // LSO_TYPE
    regex(string("^(R\\d+)\\s*,\\s*")
        + string("\\[\\s*(R\\d+)\\s*,\\s*(R\\d+)\\s*\\]$"), regex::icase)
}};

const array<vector<const char*>,FMT_LEN> FMT_EXPECTED = {{
    { "" },                 // S_TYPE
    { " Rd" },              // R1_TYPE
    { " Rd, Rn" },          // R2_TYPE
    { " Rn, Rm" },          // R2NW_TYPE
    { " Rd, Rn, Rm" },      // R3_TYPE
    { " Imm", " Label" },   // B_TYPE
    { " Rd, Imm" },         // I_TYPE
    { " Rd, Flags" },       // FL_TYPE
    { " Flags, Rn" },       // FS_TYPE 
    { " Rd, [Rn]" },        // LS_TYPE
    { " Rd, [Rn, Rm]" },    // LSO_TYPE
}};

const vector<string> RESERVED_NAMES = {
    "FLAGS"
};


/* ========================================================================= *
 * Regular Expression Definitions
 * ========================================================================= */
const regex extract_inst_re(
    "^((?:\\s*\\w*:)*)\\s*(\\w+)?\\s*([^;]*)\\s*(?:;.*)?$");
const regex label_re("(\\w*):");
const regex dec_re("^(-?\\d+)$");
const regex udec_re("^(\\d+)$");
const regex hex_re("^0[xX]([A-F\\d]+)$");
const regex bin_re("^0[bB]([01]+)$");


/* ========================================================================= *
 * Function Declarations
 * ========================================================================= */

/* ------------------------------------------------------------------------- *
 * parse_program
 * - First pass of input file, builds list of tokenized instructions
 *     and map from labels to instructions.
 * - Conditionally enforces exact syntax with `strict_parsing` flag
 *   - When false, allows spaces instead of commas between operands,
 *     and ignores missing `[]` in load-store instructions.
 * - Returns true upon succesful completion,
 *     builds `prog.insts`, `prog.label_lookup`, `prog.labels`,
 *     and `prog.debug_line_nums`.
 * - Returns false if some syntax error is found.
 * ------------------------------------------------------------------------- */
bool parse_program(ifstream& in, prog_s& prog, bool strict_parsing=false);

/* ------------------------------------------------------------------------- *
 * encode_program
 * - Second pass of input file, encodes tokenized instructions into their
 *     appropriate hex machine code representation to build `prog.mcode`.
 * - Returns true upon succesful completion.
 * - Returns false if any operand cannot be encoded.
 * ------------------------------------------------------------------------- */
bool encode_program(prog_s& prog);

/* ------------------------------------------------------------------------- *
 * write_program
 * - Writes the program to a given output file.
 * -  File should be opened before calling this function.
 * ------------------------------------------------------------------------- */
void write_program(ofstream& out, const prog_s& prog);

/* ------------------------------------------------------------------------- *
 * print_program_listing
 * - Prints more verbose program listing to standard error.
 * ------------------------------------------------------------------------- */
void print_program_listing(const prog_s& prog);

/* ------------------------------------------------------------------------- *
 * Smaller helper functions
 * ------------------------------------------------------------------------- */
// encodes an operand string into a register value
bool encode_register(const string& str, mword_t& buf);

// converts a numerically encoded instruction to a hexadecimal string
string to_hex_string(mword_t enc_inst, int bits_to_convert=WORD_SIZE);

// checks label name against mnemonics, register formats, and illegal names
bool is_reserved_name(const string& str);

/* ------------------------------------------------------------------------- *
 * IO helper functions
 * ------------------------------------------------------------------------- */
// validates and organizes command line options
bool get_options(int argc, char** argv, prog_opts_s& opts);

// prints the help message upon failure to run
void print_help();

// converts a string to uppercase
string str_to_upper(const string& str);

// trims whitespace from head of string
string& trim_head(string& str);

// trims whitespace from tail of string
string& trim_tail(string& str);

// converts a number to an ordinal string
string ordinal_str(unsigned n);

// to_string for instruction token array
string to_string(const inst_tokens_t& inst);

// print string as a line to cerr and mark the specified section underneath
void line_error_marker(const string& line, int i_start, int len);

// print instruction as a line to cerr and mark the specified token
void inst_error_marker(const inst_tokens_t& inst, unsigned opr);


/* ========================================================================= *
 * Main Function
 * ========================================================================= */
int main(int argc, char** argv) {
    // obtain arguments/options and check input file
    prog_opts_s opts;
    if(!get_options(argc, argv, opts)) {
        print_help();
        return 1;
    }

    // attempt to open input file
    ifstream fin;
    fin.open(opts.src_file);
    if(fin.fail()) {
        cerr << "Error: could not open source file '" << opts.src_file << "'" 
             << endl;
        return 1;
    }

    // initialize data structures for parsing
    prog_s prog;

    // parse source file
    bool parse_success = parse_program(fin, prog, opts.strict_flag);
    if(!parse_success) {
        cerr << "Error: failed to parse '" << opts.src_file
             << "' into valid program, aborting..." 
             << endl;
        return 1;
    }

    // close source file
    fin.close();

    // initialize data structures for encoding
    vector<mword_t> mcode;

    // encode parsed program
    bool encode_success = encode_program(prog);
    if(!encode_success) {
        cerr << "Error: failed to encode '" << opts.src_file
             << "' into valid program, aborting..." 
             << endl;
        return 1;
    }

    // attempt to open output file
    ofstream fout;
    fout.open(opts.out_file);
    if(fout.fail()) {
        cerr << "Error: could not open destination file '" << opts.out_file 
             << "'" << endl;
        return 1;
    }

    // if `-l` option enabled, write program listing
    if(opts.list_flag)
        print_program_listing(prog);

    // write encoded program to source file
    write_program(fout, prog);

    // close destination file
    fout.close();

    return 0;
}


/* ========================================================================= *
 * Function Declarations
 * ========================================================================= */

/* ------------------------------------------------------------------------- *
 * parse_program
 * - First pass of input file, builds list of tokenized instructions
 *     and map from labels to instructions.
 * - Conditionally enforces exact syntax with `strict_parsing` flag
 *   - When false, allows spaces instead of commas between operands,
 *     and ignores missing `[]` in load-store instructions.
 * - Returns true upon succesful completion,
 *     builds `prog.insts`, `prog.label_lookup`, `prog.labels`,
 *     and `prog.debug_line_nums`.
 * - Returns false if some syntax error is found.
 * ------------------------------------------------------------------------- */
bool parse_program(ifstream& fin, prog_s& prog, bool strict_parsing) {
    // set up parsing vars
    // ---------------------------------------------------------------------
    string label_buf;
    string label_raw_buf;
    string line_buf;
    string mne;
    string oprs;
    smatch m;
    smatch oprs_m;
    inst_tokens_t inst_buf;
    inst_tokens_t inst_raw_buf;
    sregex_iterator reit_begin;
    sregex_iterator reit_end;
    OPCODE inst_opcode;
    unsigned file_line = 0;

    // parse file, line by line
    // ---------------------------------------------------------------------
    while(getline(fin, line_buf)) {
        trim_head(trim_tail(line_buf));
        file_line++;

        // match line with instruction extractor
        // -----------------------------------------------------------------
        if(!regex_match(line_buf, m, extract_inst_re)) {
            cerr << "Error: line[" << file_line << "]: "
                 << "unparseable line, could not extract instruction. "
                 << "This shouldn't happen, but it did, sorry:"
                 << endl;
            line_error_marker(line_buf, 0, line_buf.size());
            return false;
        }

        // extract labels
        // -----------------------------------------------------------------
        if(m[1].length() > 0) {
            const string& labels = m[1].str();
            reit_begin = sregex_iterator(
                labels.begin(), labels.end(), label_re);
            for(auto it=reit_begin; it!=reit_end; ++it) {
                label_raw_buf = (*it)[1].str();
                label_buf = str_to_upper(label_raw_buf);

                // error: empty label
                if(label_buf.size() == 0) {
                    cerr << "Error: line[" << file_line << "]: "
                         << "expected label name before ':', "
                         << "but found empty string:"
                         << endl;
                    line_error_marker(line_buf, 
                        m.position(1)+it->position(1), 1);
                    return false;
                }

                // error: illegal label, reserved
                if(!is_reserved_name(label_buf)) {
                    cerr << "Error: line[" << file_line << "]: "
                         << "illegal label name '"
                         << label_raw_buf << "', reserved by ISA:"
                         << endl;
                    line_error_marker(line_buf, 
                        m.position(1)+it->position(1), it->length());
                    return false;
                }

                // error: invalid label (leading digit)
                if(isdigit(label_buf[0])) {
                    cerr << "Error: line[" << file_line << "]: "
                         << "invalid label name '"
                         << label_raw_buf << "', can't start with a digit:"
                         << endl;
                    line_error_marker(line_buf, 
                        m.position(1)+it->position(1), it->length());
                    return false;
                }

                // error: repeated label
                if(prog.label_lookup.count(label_buf) != 0) {
                    cerr << "Error: line[" << file_line << "]: "
                         << "repeat instance of label '"
                         << label_raw_buf << "':"
                         << endl;
                    line_error_marker(line_buf, 
                        m.position(1)+it->position(1), it->length());
                    return false;
                }

                // insert label list and lookup
                mword_t target_addr = prog.insts.size();
                prog.label_lookup[label_buf] = target_addr;
                prog.labels.push_back({ label_buf, target_addr });
            }
        }

        // extract mnemonic
        // -----------------------------------------------------------------
        // error: line content with no mnemonic
        if(m[2].length() == 0) {
            if(m[3].length() > 0) {
                cerr << "Error: line[" << file_line << "]: "
                     << "could not locate instruction mnemonic:"
                     << endl;
                line_error_marker(line_buf, 
                    m.position(3), m[3].length());
                return false;
            }
        }
        else {
            mne = m[2].str();
            string mne_key = str_to_upper(mne);

            // error: invalid mnemonic
            if(ISA.count(mne_key) == 0) {
                cerr << "Error: line[" << file_line << "]: "
                     << "invalid mnemonic '" << mne << "':"
                     << endl;
                line_error_marker(line_buf, 
                    m.position(2), m[2].length());
                return false;
            }

            // grab mnemonic configs
            const vector<OPCODE>& mne_opcodes = ISA.at(mne_key);

            // extract operands
            // -------------------------------------------------------------
            oprs = m[3];
            trim_tail(oprs);

            // iterate over and test all regex for instruction format
            bool found_matching_fmt = false;
            for(auto it=mne_opcodes.begin(); 
                    !found_matching_fmt && it!=mne_opcodes.end();
                    ++it) {
                // choose between strict and relaxed parsing
                regex fmt_re = strict_parsing 
                    ? FMT_REGEX_STRICT[OPC_TO_FMT.at(*it)] 
                    : FMT_REGEX[OPC_TO_FMT.at(*it)];

                // perform regex match
                if(regex_match(oprs, oprs_m, fmt_re)) {
                    found_matching_fmt = true;
                    inst_opcode = *it;
                }
            }

            // error: no valid operand format for mnemonic
            if(!found_matching_fmt) {
                cerr << "Error: line[" << file_line << "]: "
                     << "could not match operand format for mnemonic '"
                     << mne << "':"
                     << endl;
                line_error_marker(line_buf, m.position(3), oprs.length());
                cerr << "--- Expected " 
                     << (mne_opcodes.size() > 1 
                        ? "one of the following formats:"
                        : "the following format:") << endl;
                for(auto it=mne_opcodes.begin(); 
                        it!=mne_opcodes.end(); 
                        ++it) {
                    auto& expected_strs = FMT_EXPECTED[OPC_TO_FMT.at(*it)]; 
                    for(auto jt=expected_strs.begin(); 
                            jt!=expected_strs.end(); 
                            ++jt) {
                        cerr << "-----> " << mne << (*jt) << endl;
                    }
                }
                return false;
            }

            // extract tokenized operands from matched format
            // -------------------------------------------------------------
            inst_buf.clear();
            inst_buf.push_back(mne_key);
            inst_raw_buf.clear();
            inst_raw_buf.push_back(mne);
            for(auto it=oprs_m.begin()+1; it!=oprs_m.end(); ++it) {
                inst_buf.push_back(str_to_upper(*it));
                inst_raw_buf.push_back(*it);
            }

            // push onto instruction list
            // -------------------------------------------------------------
            prog.insts.emplace_back(inst_opcode, inst_buf);
            prog.insts_raw.push_back(inst_raw_buf);
            prog.debug_line_nums.push_back(file_line);

            // error: instruction overflow
            if(prog.insts.size() > MAX_INST) {
                cerr << "Error: line[" << file_line << "]: "
                     << "instruction count exceeds limit (" 
                     << "max = " << MAX_INST << ")" 
                     << endl;
                return false;
            }
        }
    }

    // cull out-of-bounds labels 
    // ---------------------------------------------------------------------
    int i = prog.labels.size();
    while(i>0 && prog.labels[i-1].address>=prog.insts.size())
        i--;
    prog.labels.resize(i);

    // signal success
    // ---------------------------------------------------------------------
    return true;
}

/* ------------------------------------------------------------------------- *
 * encode_program
 * - Second pass of input file, encodes tokenized instructions into their
 *     appropriate hex machine code representation to build `prog.mcode`.
 * - Returns true upon succesful completion.
 * - Returns false if any operand cannot be encoded.
 * ------------------------------------------------------------------------- */
bool encode_program(prog_s& prog) {
    // loop through parsed instruction list, encoding each instruction
    // ---------------------------------------------------------------------
    for(unsigned i=0; i<prog.insts.size(); i++) {
        // fetch instruction opcode, tokens, and format
        // -----------------------------------------------------------------
        const OPCODE inst_opcode = prog.insts[i].first;
        inst_tokens_t& inst_toks = prog.insts[i].second;
        const inst_tokens_t& inst_raw_toks = prog.insts_raw[i];
        const I_FMT inst_fmt = OPC_TO_FMT.at(inst_opcode);
        const fmt_config_t fmt_config = FMT_CONFIG[inst_fmt];

        // init encoded instruction buffer with the opcode set
        // -----------------------------------------------------------------
        mword_t enc_inst_buf = inst_opcode;

        // iterate through operands, encode them, then add them to buffer
        // -----------------------------------------------------------------
        for(unsigned o=0; o<fmt_config.size(); o++) {
            string& opr_str = inst_toks[1+o];
            string opr_str_key = str_to_upper(opr_str);
            // encode operand based on OPR_WIDTH for given instruction
            auto opr_p = fmt_config[o].first;
            auto opr_w = fmt_config[o].second;
            mword_t opr_buf = 0;
            if(opr_w == REG) {
                // encode register
                // error unknown register
                if(!encode_register(opr_str, opr_buf)) {
                    cerr << "Error: line[" << prog.debug_line_nums[i]
                         << "]: could not encode " << ordinal_str(o+1)
                         << " operand '" << opr_str
                         << "', expected register between 'r0' and 'r"
                         << MAX_REG << "':" 
                         << endl;
                    inst_error_marker(inst_raw_toks, 1+o);
                    return false;
                }
                // replace token with caps version
                // inst_toks[1+o] = opr_str_key;
            }
            else if(opr_w == IMM) {
                smatch num_m;
                long long parsed = 0;
                bool parse_success = false;
                bool parse_decimal = false;
                bool parse_label = false;
                // try to find label and compute relative branch
                if(inst_fmt == B_TYPE 
                        && prog.label_lookup.count(opr_str_key) > 0) {
                    parse_success = true;
                    parse_label = true;
                    parsed = prog.label_lookup[opr_str_key] - (i+1LL);
                }
                // try to parse as decimal
                else if(regex_match(opr_str, num_m, dec_re)) {
                    parse_success = true;
                    parse_decimal = true;
                    parsed = strtoll(num_m[1].str().c_str(), nullptr, 10);
                }
                // try to parse as hex
                else if(regex_match(opr_str, num_m, hex_re)) {
                    parse_success = true;
                    parsed = strtoll(num_m[1].str().c_str(), nullptr, 16);
                    if(num_m[1].length() > IMM_NIBS) {
                        cerr << "Error: line[" << prog.debug_line_nums[i]
                             << "]: could not encode " << ordinal_str(o+1)
                             << " operand '" << opr_str
                             << "', hex value has too many nibbles ("
                             << "max = " << IMM_NIBS << "):" 
                             << endl;
                        inst_error_marker(inst_raw_toks, 1+o);
                        return false;
                    }
                    // convert to negative
                    if(parsed&(1<<(IMM-1)))
                        parsed |= -1ULL<<IMM;
                }
                // try to parse as binary
                else if(regex_match(opr_str, num_m, bin_re)) {
                    parse_success = true;
                    parsed = strtoll(num_m[1].str().c_str(), nullptr, 2);
                    // error: too many bits
                    if(num_m[1].length() > IMM) {
                        cerr << "Error: line[" << prog.debug_line_nums[i]
                             << "]: could not encode " << ordinal_str(o+1)
                             << " operand '" << opr_str
                             << "', binary value has too many bits ("
                             << "max = " << IMM << "):" 
                             << endl;
                        inst_error_marker(inst_raw_toks, 1+o);
                        return false;
                    }
                    // convert to negative
                    if(parsed&(1<<(IMM-1)))
                        parsed |= -1ULL<<IMM;
                }

                // error: could not encode immediate
                if(!parse_success) {
                    cerr << "Error: line[" << prog.debug_line_nums[i]
                         << "]: could not encode " << ordinal_str(o+1)
                         << " operand '" << opr_str
                         << "', expected immediate value"
                         << (inst_fmt == B_TYPE ? " or valid label" : "")
                         << (inst_opcode == MOVIM ? " or register" : "")
                         << ":" 
                         << endl;
                    inst_error_marker(inst_raw_toks, 1+o);
                    return false;
                }
                // error: out of bounds immediate
                if(parsed < IMM_MIN || parsed > IMM_MAX) {
                    cerr << "Error: line[" << prog.debug_line_nums[i]
                         << "]: could not encode " << ordinal_str(o+1)
                         << " operand '" << opr_str
                         << "'" 
                         << (parse_decimal 
                                ? "" 
                                : (" (" + to_string(parsed) + ")") )
                         << ", "
                         << (parse_label
                                ? "branch offset from label "
                                : "immediate value ")
                         << "out of range ["
                         << IMM_MIN << ", " << IMM_MAX << "]:"
                         << endl;
                    inst_error_marker(inst_raw_toks, 1+o);
                    return false;
                }

                // place parsed immediate into operand buffer
                opr_buf = static_cast<mword_t>(parsed);
                
                // replace inst token with sanitized hex version
                inst_toks[1+o] = "0x" + to_hex_string(opr_buf, IMM)
                    + (parse_label ? "    " : "")
                    + " ; (" + to_string(parsed) 
                    + (parse_label ? (" -> " + opr_str_key) : "") + ")";
            }
            else { // opr_w == NON
                opr_buf = 0;
            }
            // mask opr_buf for width (shouldn't be necessary, but stay safe)
            opr_buf &= WIDTH_TO_BITS(opr_w);
            // shift opr_buf to proper position
            opr_buf <<= opr_p;
            // insert opr_buf into instruction buffer
            enc_inst_buf |= opr_buf;
        }

        // push encoded instruction
        prog.mcode.push_back(enc_inst_buf);
    }

    // signal success
    return true;
}

/* ------------------------------------------------------------------------- *
 * write_program
 * - Writes the program to a given output file.
 * -  File should be opened before calling this function.
 * ------------------------------------------------------------------------- */
void write_program(ofstream& out, const prog_s& prog) {
    out << "v2.0 raw";
    for(auto it=prog.mcode.begin(); it!=prog.mcode.end(); ++it)
        out << endl << to_hex_string(*it);
}

/* ------------------------------------------------------------------------- *
 * print_program_listing
 * - Prints more verbose program listing to standard error.
 * ------------------------------------------------------------------------- */
void print_program_listing(const prog_s& prog) {
    cerr << "=== LABEL LIST ===" << endl;
    string::size_type longest_label = 0; //5;
    for(auto it=prog.labels.begin(); it!=prog.labels.end(); ++it) {
        if(it->name.size() > longest_label)
            longest_label = it->name.size();
    }
    longest_label += 1;
    // cerr << setw(longest_label) << "LABEL"
    //      << " : ADDR" << endl
    //      << setfill('-') << setw(longest_label+1) << ""
    //      << "+"
    //      << setw(max(0UL,18-(longest_label+2))) << ""
    //      << setfill(' ') << endl;
    for(auto it=prog.labels.begin(); it!=prog.labels.end(); ++it) {
        cerr << setw(longest_label) << it->name 
             << ": 0x" << to_hex_string(it->address, IMM)
             << endl;
    }
    cerr << endl
         << "====== MACHINE PROGRAM ======" << endl
         << "  ADDR: MCODE  | ASSEMBLY    " << endl
         << "---------------+-------------" << endl;
    for(unsigned i=0; i<prog.insts.size(); i++) {
        cerr << " 0x" << to_hex_string(static_cast<mword_t>(i), IMM) << ':'
             << " 0x" << to_hex_string(prog.mcode[i]) << " | ";
        const I_FMT inst_fmt = OPC_TO_FMT.at(prog.insts[i].first);
        bool is_ls_type = inst_fmt == LS_TYPE || inst_fmt == LSO_TYPE;
        const inst_tokens_t& toks = prog.insts[i].second;
        cerr << setw(4) << setiosflags(ios_base::left) << toks.at(0);
        for(unsigned t=1; t<toks.size(); t++) {
            if(t > 1)
                cerr << ',';
            cerr << ' ';
            if(t == 2 && is_ls_type)
                cerr << '[';
            cerr << toks.at(t);
        }
        if(is_ls_type)
            cerr << ']';
        cerr << endl;
    }
}

/* ------------------------------------------------------------------------- *
 * Smaller helper functions
 * ------------------------------------------------------------------------- */
// encodes an operand string into a register value
bool encode_register(const string& str, mword_t& buf) {
    if(str.size() != 2 || toupper(str[0]) != 'R')
        return false;
    buf = str[1] - '0';
    // check for out of bounds register value
    //   negatives will overflow so no need to check `< 0`
    if(buf > MAX_REG)
        return false;
    return true;
}

// converts a numerically encoded instruction to a hexadecimal string
string to_hex_string(mword_t enc_inst, int bits_to_convert) {
    string hex;
    while(bits_to_convert > 0) {
        // grab nibble
        mword_t nibble = enc_inst&0xf;
        // encode as integer or char
        if(nibble < 10)
            hex += '0'+nibble;
        else
            hex += 'A'+(nibble-10);
        // shift to access next nibble and dec btc
        enc_inst >>= 4;
        bits_to_convert -= 4;
    }
    // return reversed string
    return string(hex.rbegin(), hex.rend());
}

// checks label name against mnemonics, register formats, and illegal names
set<string> illegal_label_cache;
bool is_reserved_name(const string& str) {
    // if first run, build cache
    if(illegal_label_cache.empty()) {
        // instruction mnemonics
        for(auto it=ISA.begin(); it!=ISA.end(); ++it)
            illegal_label_cache.insert(it->first);
        // registers
        for(unsigned i=0; i<=WIDTH_TO_BITS(REG); i++)
            illegal_label_cache.insert("R"+to_string(i));
        // hard-coded reserved names
        for(auto it=RESERVED_NAMES.begin(); it!=RESERVED_NAMES.end(); ++it)
            illegal_label_cache.insert(*it);
    }

    // verify label against illegal name cache
    return illegal_label_cache.count(str) == 0;
}

/* ------------------------------------------------------------------------- *
 * IO helper functions
 * ------------------------------------------------------------------------- */
// validates and organizes command line options
bool get_options(int argc, char** argv, prog_opts_s& opts) {
    // error: invalid number of arguments
    if(argc < 3 || argc > 6) {
        if(argc > 1)
            cerr << "Error: invalid number of arguments" << endl;
        return false;
    }

    // src and out file option is always 1st and 2nd arguments
    opts.src_file = argv[1];
    opts.out_file = argv[2];

    // loop through other arguments 
    for(int i=3; i<argc; i++) {
        // parse list_flag option
        if(strcmp(argv[i], "-l") == 0) {
            opts.list_flag = true; 
        }
        // parse strict_flag option
        else if(strcmp(argv[i], "-s") == 0) {
            opts.strict_flag = true;
        }
        else {
            cerr << "Error: unrecognized argument '" << argv[i] << "'"
                 << endl;
            return false;
        }
    }

    // signal success
    return true;
}

// prints the help message upon failure to run
void print_help() {
    cerr << "USAGE:  alarmas <source file> <object file> [-l] [-s]" << endl
         << "        -l : print listing to standard error" << endl
         << "        -s : strict parsing forces correct syntax" << endl;
}

// converts a string to uppercase
string str_to_upper(const string& str) {
    stringstream res;
    for(auto it=str.begin(); it!=str.end(); ++it)
        res << static_cast<char>(toupper(*it));
    return res.str();
}

// trims whitespace from head of string
string& trim_head(string& str) {
    int i = 0;
    while(i<str.size() && isspace(str[i]))
        i++;
    str = str.substr(i);
    return str;
}

// trims whitespace from tail of string
string& trim_tail(string& str) {
    int i = str.size();
    while(i>0 && isspace(str[i-1]))
        i--;
    str.resize(i);
    return str;
}

// converts a number to an ordinal string
string ordinal_str(unsigned n) {
    return to_string(n) + ORD_SUFXS[min(n-1u,3u)]; 
}

// to_string for instruction token array
string to_string(const inst_tokens_t& inst) {
    // assumes first token is present
    string out = inst[0];
    for(unsigned i=1; i<inst.size(); i++) {
        if(!(inst[i].empty())) {
            out += ' ';
            out += inst[i];
        }
    }
    return out;
}

// print string as a line to cerr and mark the specified section underneath
void line_error_marker(const string& line, int i_start, int len) {
    cerr << "--> " << line << endl;
    if(i_start < 0)
        i_start = 0;
    string head = line.substr(0, i_start);
    for(auto it=head.begin(); it!=head.end(); ++it) { 
        if(!isspace(*it))
            *it = ' ';
    }
    cerr << "    " << head
         << '^' << string(len==0 ? 0 : len-1, '~')
         << endl;
}

// print instruction as a line to cerr and mark the specified token
void inst_error_marker(const inst_tokens_t& inst, unsigned opr) {
    string inst_str = to_string(inst);
    unsigned i = 0;
    unsigned n = 0;
    if(opr >= MAX_OPR)
        n = inst_str.size();
    else {
        for(unsigned o=0; o<opr; o++)
            i += inst[o].size() + 1;
        n = inst[opr].size();
    }
    line_error_marker(inst_str, i, n);
}
