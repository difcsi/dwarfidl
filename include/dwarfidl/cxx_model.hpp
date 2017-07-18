/* dwarfpp: C++ binding for a useful subset of libdwarf, plus extra goodies.
 * 
 * cxx_model.hpp: tools for constructing binary-compatible C++ models 
 * 			of DWARF elements.
 *
 * Copyright (c) 2009--2012, Stephen Kell.
 */

#ifndef DWARFPP_CXX_MODEL_HPP_
#define DWARFPP_CXX_MODEL_HPP_

#include <vector>
#include <map>
#include <string>
#include <algorithm>
#include <sstream>
#include <regex>
#include <srk31/algorithm.hpp>
#include <srk31/indenting_ostream.hpp>

#include <cxxgen/cxx_compiler.hpp>
#include <dwarfpp/spec.hpp>
#include <dwarfpp/lib.hpp>

namespace dwarf {
namespace tool {

using namespace dwarf;
using dwarf::lib::Dwarf_Half;
using dwarf::lib::Dwarf_Off;
using dwarf::core::root_die;
using dwarf::core::iterator_base;
using dwarf::core::iterator_df;
using dwarf::core::basic_die;
using dwarf::core::type_die;
using dwarf::core::base_type_die;
using dwarf::core::subprogram_die;
using dwarf::core::subroutine_type_die;
using dwarf::core::formal_parameter_die;

using std::cerr;
using std::vector;
using std::pair;
using std::map;
using std::string;
using boost::optional;
using srk31::indenting_ostream;

/** This class contains generally useful stuff for generating C++ code.
 *  It should be free from DWARF details. */
class cxx_generator
{
public:
	static const vector<string> cxx_reserved_words;
	static bool is_reserved(const string& word);
	static bool is_valid_cxx_ident(const string& word);
	virtual string make_valid_cxx_ident(const string& word);
	virtual string cxx_name_from_string(const string& s, const char *prefix);
	virtual string name_from_name_parts(const vector<string>& parts);
};

/** This class implements a mapping from DWARF constructs to C++ constructs,
 *  and utility functions for understanding the C++ constructs corresponding
 *  to various DWARF elements. */
class cxx_generator_from_dwarf : public cxx_generator
{
protected:
	virtual string get_anonymous_prefix()
	{ return "_dwarfhpp_anon_"; } // HACK: remove hard-coding of this in libdwarfpp, dwarfhpp and cake
	virtual string get_untyped_argument_typename() = 0;

public:
	const spec::abstract_def *const p_spec;

	cxx_generator_from_dwarf() : p_spec(&spec::DEFAULT_DWARF_SPEC) {}
	cxx_generator_from_dwarf(const spec::abstract_def& s) : p_spec(&s) {}


	bool is_builtin(iterator_df<> p_d);

	string 
	name_for(iterator_df<type_die> t) 
	{ return local_name_for(t); }
	
	virtual 
	optional<string>
	name_for_base_type(iterator_df<base_type_die>) = 0;
	
	vector<string> 
	name_parts_for(iterator_df<type_die> t) 
	{ return local_name_parts_for(t); }

	bool 
	type_infixes_name(iterator_df<basic_die> p_d);

	string 
	local_name_for(iterator_df<basic_die> p_d,
		bool use_friendly_names = true) 
	{ return name_from_name_parts(local_name_parts_for(p_d, use_friendly_names)); }

	vector<string> 
	local_name_parts_for(iterator_df<basic_die> p_d,
		bool use_friendly_names = true);
		
	string 
	fq_name_for(iterator_df<basic_die> p_d)
	{ return name_from_name_parts(fq_name_parts_for(p_d)); }
	
	vector<string> 
	fq_name_parts_for(iterator_df<basic_die> p_d);
	
	string 
	cxx_name_from_die(iterator_df<basic_die> p_d);

	bool 
	cxx_type_can_be_qualified(iterator_df<type_die> p_d) const;

	bool 
	cxx_type_can_have_name(iterator_df<type_die> p_d) const;

	pair<string, bool>
	cxx_declarator_from_type_die(
		iterator_df<type_die> p_d, 
		optional<string> infix_typedef_name = optional<string>(),
		bool use_friendly_names = true,
		optional<string> extra_prefix = optional<string>(),
		bool use_struct_and_union_prefixes = true
	);

	bool 
	cxx_assignable_from(
		iterator_df<type_die> dest,
		iterator_df<type_die> source
	);

	bool 
	cxx_is_complete_type(iterator_df<type_die> t);
	
	pair<string, bool>
	name_for_type(
		iterator_df<type_die> p_d, 
		optional<string> infix_typedef_name = optional<string>(),
		bool use_friendly_names = true);

	string 
	name_for_argument(
		iterator_df<formal_parameter_die> p_d, 
		int argnum);

	string
	make_typedef(
		iterator_df<type_die> p_d,
		const string& name 
	);
	
	string
	make_function_declaration_of_type(
		iterator_df<subroutine_type_die> p_d,
		const string& name,
		bool write_semicolon = true,
		bool wrap_with_extern_lang = true
	);

	string 
	create_ident_for_anonymous_die(
		iterator_df<basic_die> p_d
	);

	string 
	protect_ident(const string& ident);
	
	template <Dwarf_Half Tag>
	void 
	emit_model(
		indenting_ostream& out,
		const iterator_base& i_d
	);
	
	template<typename Pred = srk31::True<iterator_df<basic_die> > > 
	void 
	dispatch_to_model_emitter(
		indenting_ostream& out, 
		const iterator_base& i_d, 
		const Pred& pred = Pred()
	);

protected:
	virtual 
	iterator_df<type_die>
	transform_type(
		iterator_df<type_die> t,
		const iterator_base& context
	)
	{
		return t;
	}

	template <typename Pred = srk31::True< iterator_df<basic_die> > >
	void 
	recursively_emit_children(
		indenting_ostream& out,
		const iterator_base& i_d,
		const Pred& pred = Pred()
	);
};

/* specializations of the above */
template<> void cxx_generator_from_dwarf::emit_model<DW_TAG_base_type>             (indenting_ostream& out, const iterator_base& i_d);
template<> void cxx_generator_from_dwarf::emit_model<DW_TAG_subprogram>            (indenting_ostream& out, const iterator_base& i_d);
template<> void cxx_generator_from_dwarf::emit_model<DW_TAG_formal_parameter>      (indenting_ostream& out, const iterator_base& i_d);
template<> void cxx_generator_from_dwarf::emit_model<DW_TAG_unspecified_parameters>(indenting_ostream& out, const iterator_base& i_d);
template<> void cxx_generator_from_dwarf::emit_model<DW_TAG_array_type>            (indenting_ostream& out, const iterator_base& i_d);
template<> void cxx_generator_from_dwarf::emit_model<DW_TAG_enumeration_type>      (indenting_ostream& out, const iterator_base& i_d);
template<> void cxx_generator_from_dwarf::emit_model<DW_TAG_member>                (indenting_ostream& out, const iterator_base& i_d);
template<> void cxx_generator_from_dwarf::emit_model<DW_TAG_pointer_type>          (indenting_ostream& out, const iterator_base& i_d);
template<> void cxx_generator_from_dwarf::emit_model<DW_TAG_reference_type>        (indenting_ostream& out, const iterator_base& i_d);
template<> void cxx_generator_from_dwarf::emit_model<DW_TAG_structure_type>        (indenting_ostream& out, const iterator_base& i_d);
template<> void cxx_generator_from_dwarf::emit_model<DW_TAG_subroutine_type>       (indenting_ostream& out, const iterator_base& i_d);
template<> void cxx_generator_from_dwarf::emit_model<DW_TAG_typedef>               (indenting_ostream& out, const iterator_base& i_d);
template<> void cxx_generator_from_dwarf::emit_model<DW_TAG_union_type>            (indenting_ostream& out, const iterator_base& i_d);
template<> void cxx_generator_from_dwarf::emit_model<DW_TAG_const_type>            (indenting_ostream& out, const iterator_base& i_d);
template<> void cxx_generator_from_dwarf::emit_model<DW_TAG_constant>              (indenting_ostream& out, const iterator_base& i_d);
template<> void cxx_generator_from_dwarf::emit_model<DW_TAG_enumerator>            (indenting_ostream& out, const iterator_base& i_d);
template<> void cxx_generator_from_dwarf::emit_model<DW_TAG_variable>              (indenting_ostream& out, const iterator_base& i_d);
template<> void cxx_generator_from_dwarf::emit_model<DW_TAG_volatile_type>         (indenting_ostream& out, const iterator_base& i_d);
template<> void cxx_generator_from_dwarf::emit_model<DW_TAG_restrict_type>         (indenting_ostream& out, const iterator_base& i_d);
template<> void cxx_generator_from_dwarf::emit_model<DW_TAG_subrange_type>         (indenting_ostream& out, const iterator_base& i_d);

	/* The dispatch function (template) defined. */
	template <typename Pred /* = srk31::True<iterator_df<basic_die> > */ > 
	void cxx_generator_from_dwarf::dispatch_to_model_emitter(
		indenting_ostream& out,
		const iterator_base& i_d,
		const Pred& pred /* = Pred() */)
	{
		// if it's a compiler builtin, skip it
		if (is_builtin(i_d)) return;
		// if it's not visible, skip it
		// if (!is_visible(p_d->get_this())) return;
		// if our predicate says no, skip it
		if (!pred(i_d)) return;
	
		// otherwise dispatch
		switch(i_d.tag_here())
		{
			case 0:
				assert(false);
			case DW_TAG_compile_unit: // we use all_compile_units so this shouldn't happen
				assert(false);
		#define CASE(fragment) case DW_TAG_ ## fragment:	\
			emit_model<DW_TAG_ ## fragment>(out, i_d); break; 
			CASE(subprogram)
			CASE(base_type)
			CASE(typedef)
			CASE(structure_type)
			CASE(pointer_type)
			CASE(volatile_type)
			CASE(formal_parameter)
			CASE(array_type)
			CASE(enumeration_type)
			CASE(member)
			CASE(subroutine_type)
			CASE(union_type)
			CASE(const_type)
			CASE(constant)
			CASE(enumerator)
			CASE(variable)
			CASE(restrict_type)
			CASE(subrange_type)   
			CASE(unspecified_parameters)
		#undef CASE
			// The following tags we silently pass over without warning
			case DW_TAG_condition:
			case DW_TAG_lexical_block:
			case DW_TAG_label:
				break;
			default:
				cerr 	<< "Warning: ignoring tag " 
							<< i_d.spec_here().tag_lookup(i_d.tag_here())
							<< endl;
				break;
		}
	}

/** This class supports generation of C++ code targetting a particular C++ compiler. */
class cxx_target : public cxx_generator_from_dwarf, public cxx_compiler
{
public:
	// forward base constructors
	cxx_target(const vector<string>& argv) : cxx_compiler(argv) {}

	cxx_target(const spec::abstract_def& s, const vector<string>& argv)
	 : cxx_generator_from_dwarf(s), cxx_compiler(argv) {}

	cxx_target(const spec::abstract_def& s)
	 : cxx_generator_from_dwarf(s) {}

	cxx_target() {}
	
	// implementation of pure virtual function in cxx_generator_from_dwarf
	optional<string> name_for_base_type(iterator_df<base_type_die> p_d);
};

} // end namespace tool
} // end namespace dwarf

#endif
