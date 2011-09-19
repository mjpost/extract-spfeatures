// nfeatures.h
//
// Mark Johnson, 12th September 2008
// 
// based on spfeatures.h, with corrections to head-finding code 
// for head-to-head dependencies suggested by Liang Huang (thanks!)
// Also added a WEdge feature which includes words and POS tags.
//
// PLEASE DO NOT MODIFY THIS FILE.  
//
// If you decide to change or define new features, make your changes
// to a copy of this file, and modify features.h to include your copy
// of this file instead of this file itself.  Also, change the
// FEATURESNICKNAME variable in the top-level Makefile to some new new
// name; that way the original model and your new model can co-exist
// (and you can compare their performance).

// Each feature is an instance of a subclass of FeatureClass.  To define
// FeatureClassPtrs{} is a class that holds one or more FeatureClass
// objects.  The standard way of interacting with FeatureClass objects
// is via the methods of FeatureClassPtrs.
//
// This version uses sptree instead of tree (sptree are annotated trees).
//
// 
#ifndef NFEATURES_H
#define NFEATURES_H

#include <algorithm>
// #include <boost/lexical_cast.hpp>
#include <cassert>
#include <cstdio>
#include <ext/hash_map>
#include <iostream>
#include <limits>
#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "lexical_cast.h"
#include "sstring.h"
#include "sp-data.h"
#include "heads.h"
#include "popen.h"
#include "sptree.h"
#include "sym.h"
#include "tree.h"
#include "utility.h"

#define FloatTol 1e-7

// SPFEATURES_COMMON_DEFINITIONS contains all of the common definitions 
// that every feature function needs.
//
#define SPFEATURES_COMMON_DEFINITIONS					\
									\
  typedef ext::hash_map<Feature,Id> Feature_Id;				\
  Feature_Id feature_id;						\
									\
  virtual void extract_features(const sp_sentence_type& s) {		\
    extract_features_helper(*this, s);					\
  }									\
									\
  virtual Id prune_and_renumber(const size_type mincount, Id nextid,	\
				std::ostream& os) {			\
    return prune_and_renumber_helper(*this, mincount, nextid, os);	\
  }									\
									\
  virtual void feature_values(const sp_sentence_type& s,		\
			      Id_Floats& p_i_v)				\
  {									\
    feature_values_helper(*this, s, p_i_v);				\
  }                                                                     \
									\
  virtual std::ostream& print_feature_ids(std::ostream& os) const {	\
    return print_feature_ids_helper(*this, os);				\
  }									\
									\
  virtual std::istream& read_feature(std::istream& is, Id id) {		\
    return read_feature_helper(*this, is, id);				\
  }


extern int debug_level;
extern bool absolute_counts;    //!< produce absolute rather than relative counts
extern bool collect_correct;    //!< collect features from correct parse
extern bool collect_incorrect;  //!< collect features from incorrect parse
extern bool lowercase_flag;     //!< lowercase all terminals when reading tree

typedef unsigned int size_type;
typedef size_type Id;           //!< type of feature Ids
#define SCANF_ID_TYPE "%u"

typedef std::map<Id,Float> Id_Float;
typedef std::vector<Id_Float> Id_Floats;

////////////////////////////////////////////////////////////////////////
//                                                                    //
//                          FeatureClass{}                            //
//                                                                    //
////////////////////////////////////////////////////////////////////////

//! FeatureClass{} is an ABC for classes of features.  A FeatureClass
//! identifies features in parse trees.  To use the template functions
//! in FeatureClass and its descendants, each subclass of
//! FeatureClass{} should define the following embedded types as
//! well as the virtual functions of FeatureClass{}.
//!
//!   Feature    -- the type of features belonging to FeatureClass
//!                   (operator << should be defined on Feature)
//!
//!   Feature_Id -- an efficient map type from features to Id
//!
//! Each FeatureClass object must also have members:
//!
//! Feature_Id feature_id;
//
class FeatureClass {
public:

  //! destructor is virtual -- put it first so it isn't forgotten!
  //
  virtual ~FeatureClass() { };

  // These next virtual functions must be implemented by any FeatureClass
  
  //! identifier() returns a unique identifying string for this
  //! FeatureClass.  By convention, a FeatureClass::identifier() consists
  //! of a major class name followed by subclass specifications, all
  //! separated by ':'.  E.g., the identifier "RFrag:2:2:5" identifes the
  //! Rule Fragment feature class with 2 ancestors and fragment lengths
  //! between 2 and 5.
  //
  virtual const char* identifier() const = 0;

  //! extract_features() extracts the relevant features from sentence s
  //
  virtual void extract_features(const sp_sentence_type& s) = 0;

  //! prune_and_renumber() prunes all features with a count of
  //!  less than mincount and renumbers them from nextid.
  //!  It returns the updated nextid.  The pruned features are
  //!  written to os.
  //
  virtual Id prune_and_renumber(const size_type mincount, Id nextid, 
				std::ostream& os) = 0;

  //! feature_values() collects the feature values for the sentence s
  //
  virtual void feature_values(const sp_sentence_type& s, Id_Floats& piv) = 0;


  //! print_feature_ids() prints out the features and their ids.
  //
  virtual std::ostream& print_feature_ids(std::ostream& os) const = 0;


  //! read_feature() reads the feature definition from in, and
  //!  sets its id to id.
  //
  virtual std::istream& read_feature(std::istream& is, Id id) = 0;


  //! define commonly used symbols
  //
  inline static symbol endmarker() { static symbol e("_"); return e; }
  inline static symbol childmarker() { static symbol c("*CHILD*"); return c; }
  inline static symbol adjunctmarker() { static symbol a("*ADJ*"); return a; }
  inline static symbol conjunctmarker() { static symbol c("*CONJ*"); return c; }
  inline static symbol headmarker() { static symbol h("*HEAD*"); return h; }
  inline static symbol lastadjunctmarker() { static symbol l("*LASTADJ*"); return l; }
  inline static symbol lastconjunctmarker() { static symbol l("*LASTCONJ*"); return l; }
  inline static symbol nonrootmarker() { static symbol r("*NONROOT*"); return r; }
  inline static symbol postheadmarker() { static symbol p("*POSTHEAD*"); return p; }
  inline static symbol preheadmarker() { static symbol p("*PREHEAD*"); return p; }

  inline static symbol DT() { static symbol dt("DT"); return dt; }
  inline static symbol NP() { static symbol np("NP"); return np; }
  inline static symbol ROOT() { static symbol root("ROOT"); return root; }
  inline static symbol S() { static symbol s("S"); return s; }
  inline static symbol SBAR() { static symbol sbar("SBAR"); return sbar; }
  inline static symbol SINV() { static symbol sinv("SINV"); return sinv; }
  inline static symbol VB() { static symbol vb("VB"); return vb; }
  inline static symbol VP() { static symbol vp("VP"); return vp; }

  inline static symbol ZERO() { static symbol zero("0"); return zero; }

  //! symbol() returns a new symbol consisting of the the nsuffixletters
  //! of s
  //
  inline static symbol suffix(symbol s, size_type nsuffixletters) {
    if (nsuffixletters == 0 || s.string_reference().size() <= nsuffixletters)
      return s;
    else
      return symbol(s.string_reference().substr(s.string_reference().size()-nsuffixletters, 
						nsuffixletters));
  }  // FeatureClass::suffix()

  //! quantize() is a utility function mapping positive ints to a
  //! small number of discrete values
  //
  inline static int quantize(int v) {
    assert(v >= 0);
    switch (v) {
    case 0: return 0;
    case 1: return 1;
    case 2: return 2;
    case 3:
    case 4: return 4;
    default: return 5;
    }
    return 5;
  }  // FeatureClass::quantize()

  //! symbol_quantize() is a utility function mapping positive ints to a
  //! small number of discrete values
  //
  inline symbol symbol_quantize(int v) {
    static symbol zero("0"), one("1"), two("2"), four("4"), five("5");
    assert(v >= 0);
    switch (v) {
    case 0: return zero;
    case 1: return one;
    case 2: return two;
    case 3:
    case 4: return four;
    default: return five;
    }
    return five;
  }  // FeatureClass::symbol_quantize()

  //! is_bounding_node() is true of nodes labeled NP, ROOT, S or SBAR
  //
  inline static bool is_bounding_node(const sptree* node) {
    return node != NULL 
      && ( node->label.cat == NP() || node->label.cat == ROOT()
	   || node->label.cat == S() || node->label.cat == SBAR() );
  }  // FeatureClass::is_bounding_node()


  // The following templated static functions do most of the actual work;
  // the specialized FeatureClasses define a few data structures, and
  // the specialized virtual functions call the templated functions below.

  //! A FeatureParseVal object defines operator[] to accumulate feature counts
  //! for the parse with id parse
  //
  template <typename FeatClass>
  struct FeatureParseVal {
    typedef typename FeatClass::Feature F;
    typedef Float V;
    typedef std::map<size_type,V> C_V;
    typedef std::map<F,C_V> F_C_V;

    size_type parse;   // parse which we are currently collecting stats from
    F_C_V  f_p_v;   // feature -> parse -> value

    V& operator[](const F& feat) { return f_p_v[feat][parse]; }
  };  // FeatureClass::FeatureParseVal{}

  //! An IdParseVal object is like a FeatureParseVal object except that
  //! it maps each feature to its Id first
  //
  template <typename FeatClass>
  struct IdParseVal {
    typedef typename FeatClass::Feature Feature;
    typedef Id F;
    typedef Float V;
    typedef std::map<size_type,V> C_V;
    typedef std::map<F,C_V> F_C_V;

    FeatClass& fc;
    size_type  parse;
    F_C_V      f_p_v;
    V	       ignored;

    IdParseVal(FeatClass& fc) : fc(fc), ignored(0) { }

    V& operator[](const Feature& f) {
      typedef typename FeatClass::Feature_Id::const_iterator It;
      It it = fc.feature_id.find(f);
      if (it != fc.feature_id.end())
	return f_p_v[it->second][parse];
      else 
	return ignored;
    }  // IdParseVal::operator[]

  };  // FeatureClass::IdParseVal{}
      
  //! sentence_parsefidvals() calls parse_featurecount() to get the
  //!  feature count for each parse, then subtracts the most common
  //!  count for each feature from each count.  This means that feature
  //!  values can be negative!  Setting absolute_counts disables this.
  //
  template <typename FeatClass, typename Fid_Parse_Val, typename Parse_Fid_Val>
  static void sentence_parsefidvals(FeatClass& fc, const sp_sentence_type& s,
				    Fid_Parse_Val& fid_parse_val,
				    Parse_Fid_Val& parse_fid_val) {

    assert(parse_fid_val.size() == s.nparses());

    fid_parse_val.f_p_v.clear();

    for (size_type i = 0; i < s.nparses(); ++i) {
      fid_parse_val.parse = i;
      fc.parse_featurecount(fc, s.parses[i], fid_parse_val);
    }

    // copy into parse_fid_val, removing pseudo-constant features

    typedef typename Fid_Parse_Val::F F;
    typedef typename Fid_Parse_Val::V V;
    typedef typename Fid_Parse_Val::C_V C_V;
    typedef typename Fid_Parse_Val::F_C_V F_C_V;
    typedef std::map<V, size_type> V_C;

    cforeach (typename F_C_V, fit, fid_parse_val.f_p_v) {
      const F& feat = fit->first;
      const C_V& parse_val = fit->second;
      if (absolute_counts) {
	for (size_type i = 0; i < s.nparses(); ++i) {
	  V val = dfind(parse_val, i);
	  if (val != 0)
	    parse_fid_val[i][feat] = val;
	}
      }
      else {  // relative counts
	V_C val_gain;  // number of times each feature value occured
	for (size_type i = 0; i < s.nparses(); ++i) {
	  const V val = dfind(parse_val, i);
	  val_gain[val] += 2;
	  val_gain[val-1] += 1;
	}
	const V& highest_gain_val = max_element(val_gain, second_lessthan())->first;
	for (size_type i = 0; i < s.nparses(); ++i) {
	  const V val = dfind(parse_val, i) - highest_gain_val;
	  if (val != 0)
	    parse_fid_val[i][feat] = val;
	}
      }
    }
  }  // FeatureClass::sentence_parsefidvals()


  //! extract_features_helper() increments by one all of the non-pseudo-constant
  //! features that occur in one or more parses of this sentence.
  //
  template <typename FeatClass>
  static void extract_features_helper(FeatClass& fc, const sp_sentence_type& s) {

    if (s.nparses() <= 1)   // ignore unambiguous sentences
      return;

    typedef FeatureParseVal<FeatClass> FPV;
    typedef typename FPV::F_C_V F_C_V;
    typedef typename FPV::C_V C_V;
    typedef typename FPV::V V;

    FPV fpv;

    for (size_type i = 0; i < s.nparses(); ++i) {
      fpv.parse = i;
      fc.parse_featurecount(fc, s.parses[i], fpv);
    }

    // trace if required
    
    if (debug_level > 1000) {
      cforeach (typename F_C_V, it, fpv.f_p_v) {
	const C_V& p_v = it->second;
	typename C_V::const_iterator it1 = p_v.find(0);
	if (it1 != p_v.end())
	  std::cerr << '\t' << fc.identifier() 
		    << '\t' << it->first 
		    << '\t' << it1->second << std::endl;
      }
    }

    // only insert non-pseudo-constant features

    cforeach (typename F_C_V, it, fpv.f_p_v) {
      const C_V& p_v = it->second;
      bool pseudoconstant = true;
      if (p_v.size() != s.nparses()) // does feature occur on
	pseudoconstant = false;      //  every parse?
      else {
	assert(!p_v.empty());
	V v0 = p_v.begin()->second;
	cforeach (typename C_V, it1, p_v) 
	  if (it1->second != v0) {
	    pseudoconstant = false;
	    break;
	  }
      }
      if (pseudoconstant == false)
	if ((collect_correct && p_v.find(0) != p_v.end())
	    || (collect_incorrect 
		&& (p_v.find(0) == p_v.end() || p_v.size() > 1)))
	++fc.feature_id[it->first];
    }
  }  // FeatureClass::extract_features_helper()


  //! print_feature_ids_helper() prints the feature_ids
  //
  template <typename FeatClass>
  static std::ostream& print_feature_ids_helper(const FeatClass& fc, std::ostream& os) {
    typedef const typename FeatClass::Feature* FeaturePtr;
    typedef std::pair<Id,FeaturePtr> IdFeaturePtr;
    typedef std::vector<IdFeaturePtr> IdFeaturePtrs;
    IdFeaturePtrs idfps;;
    idfps.reserve(fc.feature_id.size());
    cforeach (typename FeatClass::Feature_Id, it, fc.feature_id)
      idfps.push_back(IdFeaturePtr(it->second, &it->first));
    std::sort(idfps.begin(), idfps.end());
    cforeach (typename IdFeaturePtrs, it, idfps)
      os << it->first
	 << '\t' << fc.identifier() 
	 << ' ' << *it->second
	 << '\n';
    os << std::flush;    
    return os;
  }  // FeatureClass::print_feature_ids_helper()


  //! prune_and_renumber_helper() extracts all features with at
  //! least mincount count, and numbers the remaining features
  //! incrementally from nextid. 
  //
  template <typename FeatClass>
  static Id prune_and_renumber_helper(FeatClass& fc, const size_type mincount,
				      Id nextid, std::ostream& os)
  {
    typedef typename FeatClass::Feature F;
    typedef std::vector<F> Fs;
    Fs fs;

    cforeach (typename FeatClass::Feature_Id, it, fc.feature_id) 
      if (it->second >= mincount) 
	fs.push_back(it->first);

    fc.feature_id.clear();

    cforeach (typename Fs, it, fs) 
      fc.feature_id[*it] = nextid++;

    print_feature_ids_helper(fc, os);
    return nextid;
  }  // FeatureClass::prune_and_renumber_helper()

  
  //! feature_values_helper() maps a sentence to its parse_id_count vector
  //
  template <typename FeatClass>
  static void feature_values_helper(FeatClass& fc, const sp_sentence_type& s, 
				    Id_Floats& p_i_v) 
  {
    assert(p_i_v.size() == s.nparses());

    IdParseVal<FeatClass> i_p_v(fc);
    sentence_parsefidvals(fc, s, i_p_v, p_i_v);
  } // FeatureClass::feature_values_helper()


  //! read_feature_helper() reads the next feature from is, and
  //! sets its id to id.  This method reads the entire rest of the
  //! line and defines the feature accordingly.
  //
  template <typename FeatClass>
  static std::istream&
  read_feature_helper(FeatClass& fc, std::istream& is, Id id)
  {
    typedef typename FeatClass::Feature F;
    typedef typename FeatClass::Feature_Id::value_type FI;
    F f;
    is >> f;
    assert(is);
    bool inserted = fc.feature_id.insert(FI(f, id)).second;
    if (!inserted) {
      std::cerr << "## Error in spfeatures:read_feature_helper(): "
		<< "duplicate feature, id = " << id
		<< ", f = `" << f << "'" << std::endl;
      exit(EXIT_FAILURE);
    }
    return is;
  }  // FeatureClass::read_feature_helper()

};  // FeatureClass{}


////////////////////////////////////////////////////////////////////////
//                                                                    //
//                       FeatureClassPtrs{}                           //
//                                                                    //
////////////////////////////////////////////////////////////////////////

//! FeatureClassPtrs{} holds pointers to FeatureClass{} objects.
//! These objects are responsible for identifying features in trees
//! and mapping a set of parse trees for a sentence to a vector
//! of feature-counts.
//!
//! In general, one or more FeatureClass pointers will be pushed
//! onto the FeatureClassPtrs object, then extract_features()
//! is called to count how many sentences each feature occurs
//! in, then prune_and_renumber() is called to prune features
//! and assign them id numbers, and finally write_features()
//! is called to map parse trees to feature vectors.
//
class FeatureClassPtrs : public std::vector<FeatureClass*> {

private:
  struct extract_features_visitor {
    struct FeatureClassPtrs& fcps;

    extract_features_visitor(FeatureClassPtrs& fcps) : fcps(fcps) { }
    
    void operator() (const sp_sentence_type& s) {

      if (debug_level > 1000)
	std::cerr << '\n' << s.parses[0].parse << '\n' << std::endl;

      foreach (FeatureClassPtrs, it, fcps)
	(*it)->extract_features(s);
    }  // FeatureClassPtrs::extract_features_visitor::operator()

  };  // FeatureClassPtrs::extract_features_visitor{}


public:

  // ##########################################################################################

  inline void features_050902();
  inline void features_071114();
  inline void features_connll();
  inline void features_splh(bool local=false, bool nngram=false);
  inline void features_splhsuffix(size_type nsuffix=0,bool local=false);
  inline void features_wedges();
  inline void wsfeatures(bool headfeatures=false, int edgefeatures=3, bool ngram=false, bool ngramtree=false, bool rbcontext=false);
  inline void nfeatures();
  inline void sfeatures();

  //! The following load FeatureClassPtrs with various sets of features
  //
  inline FeatureClassPtrs(const char* fcname=NULL);

  //! extract_features() extracts features from the tree file infile.
  //
  void extract_features(const char* parseincmd, const char* goldincmd) {
    extract_features_visitor efv(*this);
    sp_corpus_type::map_sentences_cmd(parseincmd, goldincmd, efv, lowercase_flag);
  }  // FeatureClassPtrs::extract_features()


  //! prune_and_renumber() prunes all features that occur in less than
  //! mincount sentences, and then assigns them a number starting at 1.
  //
  Id prune_and_renumber(size_type mincount=5, std::ostream& os=std::cout) {
    Id nextid = 0;
    cforeach (FeatureClassPtrs, it, *this)
      nextid = (*it)->prune_and_renumber(mincount, nextid, os);
    return nextid;
  }  // FeatureClassPtrs::prune_and_renumber()

  
  //! write_features() maps a tree data file into a feature
  //! data file.  This is used to prepare a feature counts
  //! file from a tree data file.
  //
  void write_features(const char* parseincmd, const char* goldincmd,
		      const char* outfile) {

    const char* filesuffix = strrchr(outfile, '.');
    std::string command(strcasecmp(filesuffix, ".bz2") 
			? (strcasecmp(filesuffix, ".gz") 
			   ? "cat > " : "gzip > ")
			: "bzip2 > ");
    command += outfile;
    FILE *out = popen(command.c_str(), "w");
    if (out == NULL) {
      std::cerr << "## Error: can't popen command " << command << std::endl;
      exit(EXIT_FAILURE);
    }
    command.clear();

    FILE* parsein = popen(parseincmd, "r");

    if (parsein == NULL) {
      std::cerr << "## Error: can't popen parseincmd = " << parseincmd << std::endl;
      exit(EXIT_FAILURE);
    }

    FILE* goldin = popen(goldincmd, "r");

    if (!goldin) {
      std::cerr << "## Error: can't popen goldincmd = " << goldincmd << std::endl;
      exit(EXIT_FAILURE);
    }

    unsigned int nsentences;
    int nread = fscanf(goldin, " %u ", &nsentences);
    if (nread != 1) {
      std::cerr << "## Failed to read nsentences from " 
		<< goldincmd << std::endl;
      exit(EXIT_FAILURE);
    }
    fprintf(out, "S=%u\n", nsentences);

    sp_sentence_type sentence;
    Id_Floats p_i_v;
    for (size_type i = 0; i < nsentences; ++i) {
      if (!sentence.read(parsein, goldin, lowercase_flag)) {
	std::cerr << "## Error reading sentence " << i+1  
		  << " from \"" << parseincmd << "\" and \"" << goldincmd << "\""
		  << std::endl;
	exit(EXIT_FAILURE);
      }
      precrec_type::edges goldedges(sentence.gold);
      fprintf(out, "G=%u N=%u", goldedges.nedges(), unsigned(sentence.parses.size()));
      p_i_v.clear();                     // Clear feature-counts
      p_i_v.resize(sentence.nparses());
      cforeach (FeatureClassPtrs, it, *this)
	(*it)->feature_values(sentence, p_i_v);

      for (size_type j = 0; j < sentence.parses.size(); ++j) {
	const sp_parse_type& p = sentence.parses[j];
	precrec_type pr(goldedges, p.parse);
	fprintf(out, " P=%u W=%u", pr.ntest, pr.ncommon);
	const Id_Float& i_v = p_i_v[j];
	cforeach (Id_Float, it, i_v) 
	  if (it->second == 1)
	    fprintf(out, " " SCANF_ID_TYPE, it->first);
	  else 
	    fprintf(out, " " SCANF_ID_TYPE "=%g", it->first, it->second);
	fprintf(out, ",");
      }
      fprintf(out, "\n");
    }

    pclose(goldin);
    pclose(parsein);
    pclose(out);
  }  // FeatureClassPtrs::write_features()

  //! read_feature_ids() reads feature ids from is, and sets
  //! each feature class' feature_id hash accordingly.
  //
  Id read_feature_ids(std::istream& is) {
    typedef std::map<std::string, FeatureClass*> St_FCp;
    St_FCp fcident_fcp;
    for (iterator it = begin(); it != end(); ++it) 
      fcident_fcp[(*it)->identifier()] = *it;

    Id id, maxid = 0;
    std::string fcident;
    while (is >> id >> fcident) {
      St_FCp::const_iterator it = fcident_fcp.find(fcident);
      if (it != fcident_fcp.end()) 
	it->second->read_feature(is, id);
      else {
	std::cerr << "## Error: can't find feature identifier " << fcident
		  << " in feature list.\n"
		  << "## best-parses incompatible with feature definition data file."
		  << std::endl;
	exit(EXIT_FAILURE);
      }
      
      is.ignore(std::numeric_limits<int>::max(), '\n');
      if (id > maxid)
	maxid = id;
    }
    return maxid;
  }  // FeatureClassPtrs::read_feature_ids()

  //! best_parse() returns the best parse tree from n-best parses for a sentence
  //
  template <typename Ws>
  const tree* best_parse(const sp_sentence_type& sentence, const Ws& ws) const {
    assert(sentence.nparses() > 0);

    Id_Floats p_i_v(sentence.nparses());
    cforeach (FeatureClassPtrs, it, *this)
      (*it)->feature_values(sentence, p_i_v);

    Float max_weight = 0;
    size_type i_max = 0;
    for (size_type i = 0; i < sentence.nparses(); ++i) {
      const Id_Float& i_v = p_i_v[i];
      Float w = 0;
      cforeach (Id_Float, ivit, i_v) {
	assert(ivit->first < ws.size());
	w += ivit->second * ws[ivit->first];
      }
      if (i == 0 || w > max_weight) {
	i_max = i;
	max_weight = w;
      }
    }
    return sentence.parses[i_max].parse0;
  } // FeatureClassPtrs::best_parse()


  //! write_ranked_trees() sorts all of the trees by their conditional
  //! probability and then writes them out in sorted order.
  //
  template <typename Ws>
  std::ostream& write_ranked_trees(const sp_sentence_type& sentence, 
				   const Ws& ws, std::ostream& os) const {
    assert(sentence.nparses() > 0);

    os << sentence.nparses() << ' ' << sentence.label << std::endl;

    Id_Floats p_i_v(sentence.nparses());
    cforeach (FeatureClassPtrs, it, *this)
      (*it)->feature_values(sentence, p_i_v);

    typedef std::pair<Id,Float> IdFloat;
    typedef std::vector<IdFloat> IdFloats;

    IdFloats idweights(sentence.nparses());

    for (size_type i = 0; i < sentence.nparses(); ++i) {
      idweights[i].first = i;
      const Id_Float& i_v = p_i_v[i];
      Float w = 0;
      cforeach (Id_Float, ivit, i_v) {
	assert(ivit->first < ws.size());
	w += ivit->second * ws[ivit->first];
      }
      idweights[i].second = w;
    }

    std::sort(idweights.begin(), idweights.end(), second_greaterthan());
    
    cforeach (IdFloats, it, idweights) {
      const sp_parse_type& parse = sentence.parses[it->first];
      os << it->second << ' ' << parse.logprob << '\n';
      write_tree_noquote_root(os, parse.parse0);
      os << std::endl;
    }
    return os;
  } // FeatureClassPtrs::write_ranked_trees()

  //! write_features_debug() writes out the features associated with each parse.
  //
  template <typename Ws>
  std::ostream& write_features_debug(const sp_sentence_type& sentence,
				     const Ws& ws, std::ostream& os) const {
    assert(sentence.nparses() > 0);

    Id_Floats p_i_v(sentence.nparses());
    cforeach (FeatureClassPtrs, it, *this)
      (*it)->feature_values(sentence, p_i_v);

    for (size_type i = 0; i < sentence.nparses(); ++i) {
      const Id_Float& i_v = p_i_v[i];
      cforeach (Id_Float, ivit, i_v) {
	if (ivit->first == 0) continue;      // skip first feature
	if (ws[ivit->first] == 0) continue;  // skip features with zero weight
	os << sentence.label << ' ' 
	   << i << ' '
	   << ivit->first << ' '
	   << ivit->second << std::endl;
      }
    }
    return os;
  } // FeatureClassPtrs::write_features_debug()
};  // FeatureClassPtrs{}


std::ostream& operator<< (std::ostream& os, const FeatureClassPtrs& fcps) {
  cforeach (FeatureClassPtrs, it, fcps)
    (*it)->print_feature_ids(os);
  return os;
}  // operator<<(FeatureClassPtrs&)

////////////////////////////////////////////////////////////////////////
//                                                                    //
//                     FeatureClass{} specializations                 //
//                                                                    //
////////////////////////////////////////////////////////////////////////


//! NLogP is the - log parse probability
//!
//! The identifier is NLogP
//
class NLogP : public FeatureClass {
public:

  typedef int Feature;  // Always zero

  std::string identifier_string;

  NLogP() : identifier_string("NLogP") { }

  template <typename FeatClass, typename Feat_Count>
  void parse_featurecount(FeatClass& fc, const sp_parse_type& parse,
			  Feat_Count& feat_count) {
    feat_count[0] -= parse.logprob;
  }  // NLogP::parse_featurecount();

  // Here is the stuff that every feature needs

  virtual const char *identifier() const {
    return identifier_string.c_str();
  }  // NLogP::identifier()

  // These virtual functions just pass control to the static template functions
  //
  SPFEATURES_COMMON_DEFINITIONS;
  
}; // NLogP{}


//! NLogCondP is the - log conditional probability of the parse
//!
//! The identifier is LogCondP
//
class NLogCondProb : public FeatureClass {
public:

  typedef int Feature;  // Always zero

  std::string identifier_string;

  NLogCondProb() : identifier_string("NLogCondP") { }

  template <typename FeatClass, typename Feat_Count>
  void parse_featurecount(FeatClass& fc, const sp_parse_type& parse,
			  Feat_Count& feat_count) {
    feat_count[0] -= parse.logcondprob;
  }  // LogCondP::parse_featurecount();

  // Here is the stuff that every feature needs

  virtual const char *identifier() const {
    return identifier_string.c_str();
  }  // NLogCondP::identifier()

  // These virtual functions just pass control to the static template functions
  //
  SPFEATURES_COMMON_DEFINITIONS;
  
}; // NLogCondP{}


//! BinnedLogCondP defines features which count binned log conditional 
//!  probabilities of a parse.
//!
//! The identifier is BinnedLogCondP:nbins:base, where nbins is the number of
//! probability bins and base is the base of the log used.
//
class BinnedLogCondP : public FeatureClass {
public:
  
  typedef int Feature;  // Feature is the bin number

  int nbins;
  Float base; 
  Float log_base;
  std::string identifier_string;

  BinnedLogCondP(int nbins=7, Float base=2) 
    : nbins(nbins), base(base), log_base(log(base)),
      identifier_string("BinnedLogCondP:") {
    (identifier_string += lexical_cast<std::string>(nbins)) += ':';
    identifier_string += lexical_cast<std::string>(base);
  } // BinnedLogCondP::BinnedLogCondP()

  template <typename FeatClass, typename Feat_Count>
  void parse_featurecount(FeatClass& fc, const sp_parse_type& parse,
			  Feat_Count& feat_count) {
    int bin = std::max(1, std::min(nbins, int(-parse.logcondprob/log_base)));
    ++feat_count[bin];
  }  // BinnedLogCondP::parse_featurecount();

  // Here is the stuff that every feature needs

  virtual const char *identifier() const {
    return identifier_string.c_str();
  }  // BinnedLogCondP::identifier()

  // These virtual functions just pass control to the static template functions
  //
  SPFEATURES_COMMON_DEFINITIONS;
  
}; // BinnedLogCondP{}


//! InterpLogCondP defines features which count binned log conditional probabilities
//! of a parse.
//!
//! The identifier is InterpLogCondP:nbins:base, where nbins is the number of
//! probability bins and base is the base of the log used.
//
class InterpLogCondP : public FeatureClass {
public:
  
  typedef int Feature;  // Feature is the bin number

  int nbins;
  Float base; 
  Float log_base;
  std::string identifier_string;

  InterpLogCondP(int nbins=7, Float base=2) 
    : nbins(nbins), base(base), log_base(log(base)),
      identifier_string("InterpLogCondP:") {
    (identifier_string += lexical_cast<std::string>(nbins)) += ':';
    identifier_string += lexical_cast<std::string>(base);
  } // InterpLogCondP::InterpLogCondP()

  template <typename FeatClass, typename Feat_Count>
  void parse_featurecount(FeatClass& fc, const sp_parse_type& parse,
			  Feat_Count& feat_count) {
    int bin = std::max(1, std::min(nbins, int(-parse.logcondprob/log_base)));
    feat_count[bin] += -parse.logcondprob/log_base;
  }  // InterpLogCondP::parse_featurecount();

  // Here is the stuff that every feature needs

  virtual const char *identifier() const {
    return identifier_string.c_str();
  }  // InterpLogCondP::identifier()

  // These virtual functions just pass control to the static template functions
  //
  SPFEATURES_COMMON_DEFINITIONS;
  
}; // InterpLogCondP{}


//! A TreeFeatureClass is an ABC for classes of features where the
//! feature count for a parse is defined by the parse's tree (most
//! features are like this).
//!
//! Every subclass to TreeFeatureClass must define a method:
//!
//!  tree_featurecount(fc, tp, feat_count);
//
class TreeFeatureClass : public FeatureClass {
public:

  //! parse_featurecount() passes the tree to tree_featurecount()
  //! to analyse.
  //
  template <typename FeatClass, typename Feat_Count>
  static void parse_featurecount(FeatClass& fc, const sp_parse_type& p,
				 Feat_Count& feat_count) {
    assert(p.parse != NULL);
    fc.tree_featurecount(fc, p.parse, feat_count);
  }  // TreeFeatureClass::parse_featurecount()

};  // TreeFeatureClass{}

//! A NodeFeatureClass is an ABC for classes of features where the
//! feature count for a tree is the sum of the feature counts for
//! each node.
//!
//! Every subclass to NodeFeatureClass must define a method:
//!
//!  node_featurecount(fc, tp, feat_count);
//
class NodeFeatureClass : public TreeFeatureClass {
public:
  
  //! tree_featurecount() sums the features on each node to get
  //!  the feature count on each tree
  //
  template <typename FeatClass, typename Feat_Count>
  static void tree_featurecount(FeatClass& fc, const sptree* tp, 
				Feat_Count& feat_count) {
    assert(tp != NULL);
    fc.node_featurecount(fc, tp, feat_count);
    if (tp->is_nonterminal())
      tree_featurecount(fc, tp->child, feat_count);
    if (tp->next != NULL)
      tree_featurecount(fc, tp->next, feat_count);
  }  // NodeFeatureClass::tree_featurecount()

};  // NodeFeatureClass{}

//! A RuleFeatureClass is an ABC for classes of features
//! with rule-like features
//!
class RuleFeatureClass : public NodeFeatureClass {
public:
  
  enum annotation_level { none, pos, lexical };
  enum annotation_type { semantic, syntactic };

  RuleFeatureClass(std::string identifier_stem,   //!< Stem of identifier
		   size_type nanccats,            //!< Number of ancestor categories above trees
		   bool label_root,	          //!< Annotate with "in root context"
		   bool label_conjunct,           //!< Annotate with "belongs to conjunction"
		   annotation_level head,         //!< Amount of head annotation 
		   annotation_level functional,   //!< Amount of function word annotation
		   annotation_level all,          //!< Amount of lexical word annotation
		   annotation_type type           //!< Head type
		   ) : nanccats(nanccats), label_root(label_root), label_conjunct(label_conjunct),
		       head(head), functional(functional), all(all), type(type),
		       identifier_string(identifier_stem), 
		       max_annotation_level(std::max(head,std::max(functional,all)))  
  {
    identifier_string += ":";
    identifier_string += lexical_cast<std::string>(nanccats) + ":";
    identifier_string += lexical_cast<std::string>(label_root) + ":";
    identifier_string += lexical_cast<std::string>(label_conjunct) + ":";
    identifier_string += lexical_cast<std::string>(head) + ":";
    identifier_string += lexical_cast<std::string>(functional) + ":";
    identifier_string += lexical_cast<std::string>(all) + ":";
    identifier_string += lexical_cast<std::string>(type);
  } // RuleFeatureClass::RuleFeatureClass()

  const size_type nanccats;
  const bool label_root;
  const bool label_conjunct;
  annotation_level head;         //!< annotation on rule's head
  annotation_level functional;   //!< annotation on (projections of) functional categories
  annotation_level all;          //!< annotation on all words
  annotation_type type;          //!< syntactic or semantic annotation

  std::string identifier_string;
  const annotation_level max_annotation_level;

  typedef std::vector<symbol> Feature;
  
  //! push_child_features() pushes the features for this child node 
  //
  void push_child_features(const sptree* node, const sptree* parent, Feature& f,
			   annotation_level& highest_level) 
  {
    const sptree* parent_headchild
      = (type == semantic 
	 ? parent->label.semantic_headchild : parent->label.syntactic_headchild);
    bool is_headchild = (node == parent_headchild);

    const sptree_label& label = node->label;
    f.push_back(label.cat);
    const sptree* lexhead 
      = (type == semantic ? label.semantic_lexhead : label.syntactic_lexhead);
    if (lexhead == NULL)
      return;
    if (all < pos 
	&& (!lexhead->is_functional() || functional < pos)
	&& (!is_headchild || head < pos))
      return;
    if (lexhead != node) {
      f.push_back(headmarker());
      f.push_back(lexhead->label.cat);
      highest_level = std::max(highest_level, pos);
    }
    if (all < lexical 
	&& (!lexhead->is_functional() || functional < lexical)
	&& (!is_headchild || head < lexical))
      return;
    f.push_back(lexhead->child->label.cat);
    highest_level = std::max(highest_level, lexical);
  }  // RuleFeatureClass::push_child_features()

  //! push_ancestor_features() pushes features for ancestor nodes.
  //
  void push_ancestor_features(const sptree* node, Feature& f) {

    f.push_back(endmarker());
    
    const sptree* parent = node->label.parent;
    for (size_type i = 0; i <= nanccats && parent != NULL; ++i) {
      f.push_back(node->label.cat);
      if (label_conjunct && parent != NULL) {
	if (parent->is_coordination()) 
	  f.push_back(parent->is_last_nonpunctuation() 
		      ? lastconjunctmarker() : conjunctmarker());
	else if (parent->is_adjunction())
	  f.push_back(parent->is_last_nonpunctuation()
		      ? lastadjunctmarker() : adjunctmarker());
      }
      node = parent;
      parent = node->label.parent;
    }
    
    if (label_root)
      for (node = parent; node != NULL; node = node->label.parent) 
	if (is_bounding_node(node) && !is_bounding_node(node->label.parent)) {
	  f.push_back(nonrootmarker());
	  break;
	}
  }  // RuleFeatureClass::push_top_ancestor_features()

  virtual const char *identifier() const {
    return identifier_string.c_str();
  }  // RuleFeatureClass::identifier()

};  // RuleFeatureClass{}

//! Identifier is Rule:<nanctrees>:<nanccats>:<root>:<conj>:<head>:<functional>:<all>:<type>
//!
class Rule : public RuleFeatureClass {
public:
  
  Rule(size_type nanctrees = 0,         //!< Number of ancestor local trees
       size_type nanccats = 0,          //!< Number of ancestor categories above trees
       bool label_root = false,	        //!< Annotate with "in root context"
       bool label_conjunct = false,     //!< Annotate with "belongs to conjunction"
       annotation_level head = none,    //!< Amount of head annotation 
       annotation_level functional = none, //!< Amount of function word annotation
       annotation_level all = none,     //!< Amount of lexical word annotation
       annotation_type type = syntactic
       ) : RuleFeatureClass(std::string("Rule:")+lexical_cast<std::string>(nanctrees), 
			    nanccats, label_root, label_conjunct, head, 
			    functional, all, type),
	   nanctrees(nanctrees) { } // Rule::Rule()

  size_type nanctrees;

  template <typename FeatClass, typename Feat_Count>
  void node_featurecount(FeatClass& fc, const sptree* node, 
			 Feat_Count& feat_count) {

    if (!node->is_nonterminal())
      return;

    Feature f;
    annotation_level highest_level = none;

    // push (possibly lexicalized) children
    
    for (const sptree* child = node->child; child != NULL; child = child->next) 
      push_child_features(child, node, f, highest_level);

    // push (possibly lexicalized) ancestor rules

    for (size_type i = 0; i < nanctrees && node->label.parent != NULL; ++i) {
      f.push_back(endmarker());
      for (const sptree* child = node->label.parent->child; child != NULL; 
	   child = child->next)
	if (child == node) {
	  f.push_back(childmarker());
	  f.push_back(child->label.cat);
	}
	else
	  push_child_features(child, node, f, highest_level);
      node = node->label.parent;
    }
    
    if (highest_level != max_annotation_level)
      return;

    push_ancestor_features(node, f);
    ++feat_count[f];
  }  // Rule::node_featurecount()

  // Here is the stuff that every feature needs

  SPFEATURES_COMMON_DEFINITIONS;
};  // Rule{}

//! NGram
//!
//! Identifier is NGram:<frag_len>:<nanccats>:<root>:<conj>:<head>:<functional>:<all>:<type>
//
class NGram : public RuleFeatureClass {
public:

  NGram(size_type fraglen = 3,           //!< Number of children in sequence
	size_type nanccats = 1,          //!< Number of ancestor categories above trees
	bool label_root = false,	 //!< Annotate with "in root context"
	bool label_conjunct = false,     //!< Annotate with "belongs to conjunction"
	annotation_level head = none,    //!< Amount of head annotation 
	annotation_level functional = none, //!< Amount of function word annotation
	annotation_level all = none,     //!< Amount of lexical word annotation
	annotation_type type = syntactic
	) : RuleFeatureClass(std::string("NGram:")+lexical_cast<std::string>(fraglen), 
			     nanccats, label_root, label_conjunct, head, functional, 
			     all, type), 
	    fraglen(fraglen) { }

  size_type fraglen;

  template <typename FeatClass, typename Feat_Count>
  void node_featurecount(FeatClass& fc, const sptree* node, Feat_Count& feat_count) 
  {
    if (!node->is_nonterminal())
      return;

    size_type nchildren = 0;
    for (const sptree* child = node->child; child != NULL; child = child->next)
      ++nchildren;

    if (nchildren < fraglen)
      return;

    const sptree* headchild = (type == semantic 
			       ? node->label.semantic_headchild 
			       : node->label.syntactic_headchild);

    typedef std::vector<const sptree*> Tptrs;
    Tptrs children;

    children.push_back(NULL);
    for (const sptree* child = node->child; child != NULL; child = child->next) 
      children.push_back(child);
    children.push_back(NULL);

    symbol headposition = preheadmarker();

    for (size_type start = 0; start+fraglen <= children.size(); ++start) {
      if (children[start] == headchild)
	headposition = postheadmarker();

      Feature f;
      annotation_level highest_level = none;
      bool includes_headchild = false;

      for (size_type pos = start; pos < start+fraglen; ++pos) {
	const sptree* child = children[pos];
	if (child != NULL) 
	  push_child_features(child, node, f, highest_level);
	else
	  f.push_back(endmarker());
	if (child == headchild)
	  includes_headchild = true;
      }

      f.push_back(headposition);

      if (includes_headchild == false && head != none)
	push_child_features(headchild, node, f, highest_level);

      if (highest_level != max_annotation_level)
	return;
      
      push_ancestor_features(node, f);

      ++feat_count[f];
    }
  }  // NGram::node_featurecount()
 
  SPFEATURES_COMMON_DEFINITIONS;
};  // NGram{}

//! NNGram
//!
//! Identifier is NNGram:<fraglen>:<headdir>:<headdist>:<nanccats>:<root>:<conj>:<head>:<functional>:<all>:<type>
//
class NNGram : public RuleFeatureClass {
public:

  NNGram(size_type fraglen = 3,           //!< Number of children in sequence
	size_type nanccats = 1,          //!< Number of ancestor categories above trees
	bool label_root = false,	 //!< Annotate with "in root context"
	bool label_conjunct = false,     //!< Annotate with "belongs to conjunction"
	annotation_level head = none,    //!< Amount of head annotation 
	annotation_level functional = none, //!< Amount of function word annotation
	annotation_level all = none,     //!< Amount of lexical word annotation
	annotation_type type = syntactic,   //!< Type of head to use
	bool headdir = false,            //!< Annotate direction to head
	bool headdist = false            //!< Annotate distance from head
	) : RuleFeatureClass(std::string("NNGram:")+lexical_cast<std::string>(fraglen)
			     +":"+lexical_cast<std::string>(headdir)+":"+lexical_cast<std::string>(headdist), 
			     nanccats, label_root, label_conjunct, head, functional, 
			     all, type), 
	    fraglen(fraglen), headdir(headdir), headdist(headdist) { }

  size_type fraglen;
  bool headdir, headdist;

  template <typename FeatClass, typename Feat_Count>
  void node_featurecount(FeatClass& fc, const sptree* node, Feat_Count& feat_count) 
  {
    if (!node->is_nonterminal())
      return;

    const sptree* headchild = (type == semantic 
			       ? node->label.semantic_headchild 
			       : node->label.syntactic_headchild);

    size_type nchildren = 0;
    size_type headlocation = 0;      //!< location of head in sequence of children
    for (const sptree* child = node->child; child != NULL; child = child->next) {
      if (child == headchild)
	headlocation = nchildren;
      ++nchildren;
    }
      
    if (nchildren+1 < fraglen)
      return;

    typedef std::vector<const sptree*> Tptrs;
    Tptrs children;
    children.push_back(NULL);
    for (const sptree* child = node->child; child != NULL; child = child->next) 
      children.push_back(child);
    children.push_back(NULL);

    symbol headposition = preheadmarker();

    for (size_type start1 = 0; start1+fraglen <= children.size(); ++start1) {
      if (children[start1] == headchild)
	headposition = postheadmarker();

      Feature f;
      annotation_level highest_level = none;
      bool includes_headchild = false;

      for (size_type pos1 = start1; pos1 < start1+fraglen; ++pos1) {
	const sptree* child = children[pos1];
	if (child != NULL) {
	  push_child_features(child, node, f, highest_level);
	  if (child == headchild)
	    includes_headchild = true;
	}
	else
	  f.push_back(endmarker());
      }

      if (headdir) {
	if (includes_headchild) {
	  assert(headlocation+1 >= start1);
	  f.push_back(symbol_quantize(headlocation+1-start1));
	}
	else
	  f.push_back(headposition);
      }

      if (headdist) {
	if (headlocation+1 < start1)
	  f.push_back(symbol_quantize(start1-headlocation-1));
	else if (headlocation+1 >= start1+fraglen) {
	  assert(headlocation + 2 > start1+fraglen);
	  f.push_back(symbol_quantize(headlocation + 2 - (start1+fraglen)));
	}
	else
	  f.push_back(symbol_quantize(0));
      }

      if (head != none) {
	if (headchild != NULL)
	  push_child_features(headchild, node, f, highest_level);
	else
	  f.push_back(headmarker());
      }

      if (highest_level != max_annotation_level)
	return;
      
      push_ancestor_features(node, f);

      ++feat_count[f];
    }
  }  // NNGram::node_featurecount()
 
  SPFEATURES_COMMON_DEFINITIONS;
};  // NNGram{}
  

//! Word{} collects information on words in their vertical context.
//!
//! Identifier is Word:<nanccats>
//!
class Word : public NodeFeatureClass {
public:
  
  Word(size_type nanccats = 1)        //!< Number of ancestor local trees
    : nanccats(nanccats), identifier_string("Word:")
  {
    identifier_string += lexical_cast<std::string>(nanccats);
  } // Word::Word()

  const size_type nanccats;
  std::string identifier_string;

  typedef std::vector<symbol> Feature;
  
  template <typename FeatClass, typename Feat_Count>
  void node_featurecount(FeatClass& fc, const sptree* node, 
			 Feat_Count& feat_count) {

    if (!node->is_preterminal())
      return;
    
    Feature f;
    f.push_back(node->child->label.cat);

    for (size_type i = 0; i < nanccats; ++i) {
      if (node == NULL)
	return;
      f.push_back(node->label.cat);
      node = node->label.parent;
    }
    
    ++feat_count[f];
  }  // Word::node_featurecount()

  // Here is the stuff that every feature needs

  virtual const char *identifier() const {
    return identifier_string.c_str();
  }  // Word::identifier()

  SPFEATURES_COMMON_DEFINITIONS;
};  // Word{}


//! WProj{} collects information on words in their vertical context.
//! It projects each word up to its maximal projection.
//!
//! Identifier is WProj:<HeadType>:<IncludeNonMaximal>:<NAncs>
//!
class WProj : public NodeFeatureClass {
public:

  enum annotation_type { semantic, syntactic };
  
  WProj(annotation_type type = semantic,  //!< Head type
	bool include_nonmaximal = false,  //!< Only include maximal projection
	size_type nancs = 1)                 //!< Extra ancestors to include
    : type(type), include_nonmaximal(include_nonmaximal), nancs(nancs), 
      identifier_string("WProj:")
  {
    (identifier_string += lexical_cast<std::string>(type)) += ':';
    (identifier_string += lexical_cast<std::string>(include_nonmaximal)) += ':';
    identifier_string += lexical_cast<std::string>(nancs);
  } // WProj::WProj()

  annotation_type type;
  bool include_nonmaximal;
  size_type nancs;
  std::string identifier_string;

  typedef std::vector<symbol> Feature;
  
  template <typename FeatClass, typename Feat_Count>
  void node_featurecount(FeatClass& fc, const sptree* node, 
			 Feat_Count& feat_count) {

    if (node->is_punctuation() || !node->is_preterminal())
      return;
    
    Feature f;
    f.push_back(node->child->label.cat);

    while (node->label.parent) {
      const sptree* parent = node->label.parent;
      const sptree* parent_headchild
	= (type == semantic 
	   ? parent->label.semantic_headchild : parent->label.syntactic_headchild);
      bool is_headchild = (node == parent_headchild && !parent->is_root());
      if (is_headchild) {
	if (include_nonmaximal)
	  f.push_back(node->label.cat);
      }
      else
	break;
      node = parent;
    }
    
    for (size_type i = 0; node != NULL && i <= nancs; node = node->label.parent, ++i) 
      f.push_back(node->label.cat);
      
    ++feat_count[f];
  }  // WProj::node_featurecount()

  // Here is the stuff that every feature needs

  virtual const char *identifier() const {
    return identifier_string.c_str();
  }  // WProj::identifier()


  SPFEATURES_COMMON_DEFINITIONS;
};  // WProj{}


//! The RightBranch defines two features: 1, which is true
//! of lexical nodes on the right-most branch and 0, which is true of
//! nodes that are not on the right-most branch.
//
class RightBranch : public TreeFeatureClass {
public:

  //! tree_featurecount() counts the number of nodes on a right branch
  //
  template <typename FeatClass, typename Feat_Count>
  static void tree_featurecount(FeatClass& fc, const sptree* tp, 
				Feat_Count& feat_count) {
    rightbranch_count(tp, 1, feat_count);
  }  // RightBranch::tree_featurecount()

  //! rightbranch_count() is called with rightmost==1 iff the parent was
  //! on the rightmost branch.
  //
  template <typename Feat_Count>
  static int rightbranch_count(const sptree* tp, int rightmost, Feat_Count& fc) {
    if (tp->next) 
      rightmost = rightbranch_count(tp->next, rightmost, fc);
    if (tp->is_punctuation())
      return rightmost;
    ++fc[rightmost];
    if (tp->is_nonterminal())
      rightbranch_count(tp->child, rightmost, fc);
    return 0;
  }  // RightBranch::rightbranch_count()

  // required types

  typedef int Feature;    // 0 = non-rightmost branch, 1 = rightmost branch

  virtual const char* identifier() const {
    static char p[] = "RightBranch";
    return p;
  }  // RightBranch::identifier()

  SPFEATURES_COMMON_DEFINITIONS;
}; // RightBranch{}


//! LeftBranchLength is a feature whose value is the log of 
//! the length of the left-branching chain terminating in
//! each preterminal.
//!
//! Identifier is LeftBranchLength
//
class LeftBranchLength : public TreeFeatureClass {
public:

  //! tree_featurecound() counts the length of a rightmost branching chain
  //
  template <typename FeatClass, typename Feat_Count>
  static void tree_featurecount(FeatClass& fc, const sptree* tp, 
				Feat_Count& feat_count) {
    leftbranch_count(tp, 1, feat_count);
  }  // RightBranch::tree_featurecount()

  //! leftbranch_count() is called with the number of left branching
  //! nodes above this one
  //
  template <typename Feat_Count>
  static void leftbranch_count(const sptree* tp, int leftmost, Feat_Count& fc) {
    if (tp==NULL)
      return;
    else if (tp->is_punctuation())
      leftbranch_count(tp->next, leftmost, fc);
    else {
      if (tp->is_preterminal()) {
	assert(leftmost >= 1);
	int log2_leftmost = int(log2f(float(leftmost)));
	++fc[log2_leftmost];
      }
      else
	leftbranch_count(tp->child, leftmost+1, fc);
      leftbranch_count(tp->next, 1, fc);
    }
  }  // LeftBranchLength::leftbranch_count()

  // required types

  typedef int Feature;    // log2 length of right branch

  virtual const char* identifier() const {
    static char p[] = "LeftBranchLength";
    return p;
  }  // LeftBranchLength::identifier()

  SPFEATURES_COMMON_DEFINITIONS;
}; // LeftBranchLength{}


//! RightBranchLength is a feature whose value is the log of 
//! the length of the right-branching chain terminating in
//! each preterminal.
//!
//! Identifier is RightBranchLength
//
class RightBranchLength : public TreeFeatureClass {
public:

  //! tree_featurecound() counts the length of a rightmost branching chain
  //
  template <typename FeatClass, typename Feat_Count>
  static void tree_featurecount(FeatClass& fc, const sptree* tp, 
				Feat_Count& feat_count) {
    rightbranch_count(tp, 1, feat_count);
  }  // RightBranch::tree_featurecount()

  //! rightbranch_count() is called with the number of right branching
  //! nodes above this one
  //
  template <typename Feat_Count>
  static int rightbranch_count(const sptree* tp, int rightmost, Feat_Count& fc) {
    if (tp->next) 
      rightmost = rightbranch_count(tp->next, rightmost, fc);
    if (tp->is_punctuation())
      return rightmost;
    if (tp->is_preterminal()) {
      assert(rightmost >= 1);
      int log2_rightmost = int(log2f(float(rightmost)));
      ++fc[log2_rightmost];
    }
    else // tp->is_nonterminal()
      rightbranch_count(tp->child, rightmost+1, fc);
    return 1;
  }  // RightBranchLength::rightbranch_count()

  // required types

  typedef int Feature;    // log2 length of right branch

  virtual const char* identifier() const {
    static char p[] = "RightBranchLength";
    return p;
  }  // RightBranchLength::identifier()

  SPFEATURES_COMMON_DEFINITIONS;
}; // RightBranchLength{}


//! RBContext
//!
//! Identifier is RBContext:<conjunct>:<parent>:<governor>:<type>
//
class RBContext : public NodeFeatureClass {
public:
  typedef std::vector<symbol> Feature;

  enum head_type_type { syntactic, semantic };

  const bool label_coordination;
  const bool label_parent;
  const bool label_governor;
  const head_type_type head_type;

  std::string identifier_string;


  RBContext(bool label_coordination = false, //!< Annotate with "is coordination"
	    bool label_parent = false,       //!< Annotate with parent's category
	    bool label_governor = false,     //!< Annotate with governor's category
	    head_type_type head_type = syntactic
	    ) 
    : label_coordination(label_coordination), label_parent(label_parent),
      label_governor(label_governor), head_type(head_type),
      identifier_string("RBContext:")
  {
    identifier_string += lexical_cast<std::string>(label_coordination) + ":";
    identifier_string += lexical_cast<std::string>(label_parent) + ":";
    identifier_string += lexical_cast<std::string>(label_governor) + ":";
    identifier_string += lexical_cast<std::string>(head_type);
  }  // RBContext::RBContext()

  const sptree* headchild(const sptree* node) const {
    return head_type == semantic 
      ? node->label.semantic_headchild : node->label.syntactic_headchild;
  }  // RBContext::headchild();

  const sptree* lexhead(const sptree* node) const {
    return head_type == semantic 
      ? node->label.semantic_lexhead : node->label.syntactic_lexhead;
  }  // RBContext::lexhead();
    
    
  template <typename FeatClass, typename Feat_Count>
  void node_featurecount(FeatClass& fc, const sptree* node, Feat_Count& feat_count) 
  {
    if (!node->is_nonterminal())
      return;

    const sptree* hchild = headchild(node);
    if (hchild == NULL)
      return;
    const sptree* lhchild = lexhead(hchild);
    if (lhchild == NULL)
      return;

    Feature f;
    if (label_coordination && node->is_coordination())
      f.push_back(conjunctmarker());

    if (label_parent)
      f.push_back(node->label.cat);

    if (label_governor) {
      f.push_back(hchild->label.cat);
      f.push_back(symbol_quantize(hchild->label.right - lhchild->label.right));
    }

    for (const sptree* child = node->child; child != NULL; child = child->next) {
      if (child == hchild) {
	f.push_back(postheadmarker());
	continue;
      }
      const sptree* lchild = lexhead(child);
      if (lchild == NULL)
	continue;
      f.push_back(child->label.cat);
      f.push_back(symbol_quantize(child->label.right - lchild->label.right));
      ++feat_count[f];
      f.pop_back();
      f.pop_back();
    }
  }
 
  // Here is the stuff that every feature needs

  //! The identifier string is RBContext:<conjunct>:<parent>:<governor>:<type>
  //
  virtual const char *identifier() const {
    return identifier_string.c_str();
  }  // RBContext::identifier()

  SPFEATURES_COMMON_DEFINITIONS;
};  // RBContext{}


//! Heads is a feature of n levels of head-to-head dependencies.
//! Heads takes special care to follow head dependencies through
//! conjunctions.
//!
//! The identifier string is Heads:nheads:governorlex:dependentlex:headtype.
//
class Heads : public NodeFeatureClass {
public:

  typedef std::vector<symbol> Feature;

  enum head_type_type { syntactic, semantic };

  const size_type nheads;      //!< number of levels of heads to use
  const bool governorlex;   //!< use governor's word (in addition to its POS)
  const bool dependentlex;  //!< use dependent's head word (in addition its POS)
  const head_type_type head_type; //!< type of head dependency to track 
  std::string identifier_string;

  Heads(size_type nheads = 2,        //!< number of levels of heads to use
	bool governorlex = true,  //!< use governor's word (in addition to its POS)
	bool dependentlex = true, //!< use dependent's head word (in addition to its POS)
	head_type_type head_type = syntactic)
    : nheads(nheads), governorlex(governorlex), dependentlex(dependentlex),
      head_type(head_type), identifier_string("Heads:") 
  { 
    identifier_string += lexical_cast<std::string>(nheads) + ":";
    identifier_string += lexical_cast<std::string>(governorlex) + ":";
    identifier_string += lexical_cast<std::string>(dependentlex) + ":";
    identifier_string += lexical_cast<std::string>(head_type);
  }  // Heads::Heads()

  const sptree* headchild(const sptree* node) const {
    return head_type == semantic 
      ? node->label.semantic_headchild : node->label.syntactic_headchild;
  }  // Heads::headchild();

  //! node_featurecount() uses headchild() to find all of the heads
  //! of this node and its ancestors.
  //
  template <typename FeatClass, typename Feat_Count>
  void node_featurecount(FeatClass& fc, const sptree* node, 
			 Feat_Count& feat_count) 
  {
    if (!node->is_preterminal())  // only consider preterminal heads
      return;
    
    Feature f;
    f.push_back(node->label.cat);
    if (dependentlex)
      f.push_back(node->child->label.cat);
    
    visit_ancestors(feat_count, node, 1, f);
    assert(f.size() == (dependentlex ? 2 : 1));
  }  // Heads::node_featurecount()

  //! visit_ancestors() is written in continuation-passing style, in order
  //! to enumerate all possible governors.
  //
  template <typename Feat_Count>
  void visit_ancestors(Feat_Count& feat_count, const sptree* node,
		       size_type nsofar, Feature& f) {
    if (nsofar == nheads) {  // are we done?
      ++feat_count[f];
      return;
    }

    const sptree* ancestor = node->label.parent;
    if (ancestor == NULL)
      return;     // no more ancestors, so we can't find enough governors

    if (ancestor->is_coordination())    
      visit_ancestors(feat_count, ancestor, nsofar, f);  // skip this level
    else {
      const sptree* hchild = headchild(ancestor);
      if (hchild != NULL && node != hchild) 
	visit_descendants(feat_count, ancestor, nsofar, f, hchild);
      else
	visit_ancestors(feat_count, ancestor, nsofar, f);
    }
  }  // Heads::visit_ancestors()

  //! visit_descendants() collects the head(s) of head and then visits ancestors
  //
  template <typename Feat_Count>
  void visit_descendants(Feat_Count& feat_count, const sptree* ancestor,
			 size_type nsofar, Feature& f, const sptree* head)
  {
    if (head->is_preterminal()) {
      f.push_back(head->label.cat);      // push governor label
      if (governorlex)
	f.push_back(head->child->label.cat);
      visit_ancestors(feat_count, ancestor, nsofar+1, f);        // visit ancestors
      f.pop_back();   // pop governor
      if (governorlex)
	f.pop_back();
    }
    else {
      const sptree* hchild = headchild(head);  
      if (head->is_coordination()) {  // all children count as heads
	for (const sptree* child = head->child; child != NULL; child = child->next) 
	  if (child->label.cat == head->label.cat || (hchild != NULL && child->label.cat == hchild->label.cat))
	    visit_descendants(feat_count, ancestor, nsofar, f, child);
      }
      else {    // visit head child
	if (hchild != NULL)
	  visit_descendants(feat_count, ancestor, nsofar, f, hchild);
      }
    }
  }  // Heads::visit_descendants()

  // Here is the stuff that every feature needs

  //! The identifier string is Heads:nheads:governorlex:dependentlex:headtype.
  //
  virtual const char *identifier() const {
    return identifier_string.c_str();
  }  // Heads::identifier()

  SPFEATURES_COMMON_DEFINITIONS;
};  // Heads{}

//! WSHeads is a feature of n levels of head-to-head dependencies.
//! Heads takes special care to follow head dependencies through
//! conjunctions.
//!
//! The identifier string is WSHeads:nsuffixletters:distribute:nheads:governorinfo:dependentinfo:headtype.
//
class WSHeads : public NodeFeatureClass {
public:

  typedef std::vector<symbol> Feature;

  enum head_type_type { syntactic, semantic };
  enum info_type { pos, closedclass, lexical };

  const size_type nsuffixletters; //!< keep nsuffix letters from words
  const bool distribute;          //!< distribute head dependencies over coordinate phrases
  const size_type nheads;         //!< number of levels of heads to use
  const info_type governorinfo;   //!< use governor's word (in addition to its POS)
  const info_type dependentinfo;  //!< use dependent's head word (in addition its POS)
  const head_type_type head_type; //!< type of head dependency to track 
  std::string identifier_string;

  WSHeads(size_type nsuffixletters=0,        //!< keep nsuffixletters from words
	  bool distribute = false,           //!< distribute head dependencies over coordinate phrases
	  size_type nheads = 2,              //!< number of levels of heads to use
	  info_type governorinfo = lexical,  //!< use governor's word (in addition to its POS)
	  info_type dependentinfo = lexical, //!< use dependent's head word (in addition to its POS)
	  head_type_type head_type = syntactic)
    : nsuffixletters(nsuffixletters), distribute(distribute), nheads(nheads), governorinfo(governorinfo), 
      dependentinfo(dependentinfo), head_type(head_type), identifier_string("WSHeads:") 
  { 
    identifier_string += lexical_cast<std::string>(nsuffixletters) + ":";
    identifier_string += lexical_cast<std::string>(distribute) + ":";
    identifier_string += lexical_cast<std::string>(nheads) + ":";
    identifier_string += lexical_cast<std::string>(governorinfo) + ":";
    identifier_string += lexical_cast<std::string>(dependentinfo) + ":";
    identifier_string += lexical_cast<std::string>(head_type);
  }  // WSHeads::WSHeads()

  const sptree* headchild(const sptree* node) const {
    return head_type == semantic 
      ? node->label.semantic_headchild : node->label.syntactic_headchild;
  }  // WSHeads::headchild();

  //! node_featurecount() uses headchild() to find all of the heads
  //! of this node and its ancestors.
  //
  template <typename FeatClass, typename Feat_Count>
  void node_featurecount(FeatClass& fc, const sptree* node, 
			 Feat_Count& feat_count) 
  {
    if (!node->is_preterminal())  // only consider preterminal heads
      return;
    
    Feature f;
    f.push_back(node->label.cat);
    if (dependentinfo == closedclass)
      f.push_back(node->child->label.cat);
    else if (dependentinfo == lexical)
      f.push_back(suffix(node->child->label.cat, nsuffixletters));
    
    visit_ancestors(feat_count, node, 1, f);
  }  // WSHeads::node_featurecount()

  //! visit_ancestors() is written in continuation-passing style, in order
  //! to enumerate all possible governors.
  //
  template <typename Feat_Count>
  void visit_ancestors(Feat_Count& feat_count, const sptree* node,
		       size_type nsofar, Feature& f) {
    if (nsofar == nheads) {  // are we done?
      ++feat_count[f];
      return;
    }

    const sptree* ancestor = node->label.parent;
    if (ancestor == NULL)
      return;     // no more ancestors, so we can't find enough governors

    if (ancestor->is_coordination()) {       // skip this level
      if (distribute || node->next == NULL)  // if !distribute, don't go up if we aren't on right branch
	visit_ancestors(feat_count, ancestor, nsofar, f);
    }
    else {
      const sptree* hchild = headchild(ancestor);
      if (hchild != NULL && node != hchild) 
	visit_descendants(feat_count, ancestor, nsofar, f, hchild);
      else
	visit_ancestors(feat_count, ancestor, nsofar, f);
    }
  }  // WSHeads::visit_ancestors()

  //! visit_descendants() collects the head(s) of head and then visits ancestors
  //
  template <typename Feat_Count>
  void visit_descendants(Feat_Count& feat_count, const sptree* ancestor,
			 size_type nsofar, Feature& f, const sptree* head)
  {
    if (head->is_preterminal()) {
      unsigned oldfsize = f.size();
      f.push_back(head->label.cat);      // push governor label
      if (governorinfo == closedclass)
	f.push_back(head->child->label.cat);
      else if (governorinfo == lexical)	  
	f.push_back(suffix(head->child->label.cat, nsuffixletters));
      visit_ancestors(feat_count, ancestor, nsofar+1, f);        // visit ancestors
      f.resize(oldfsize);   // pop annotations we just pushed
    }
    else {
      if (head->is_coordination() && distribute) {  // all children count as heads
	for (const sptree* child = head->child; child != NULL; child = child->next) 
	  if (child->label.cat == head->label.cat)
	    visit_descendants(feat_count, ancestor, nsofar, f, child);
      }
      else {    // visit head child
	const sptree* child = headchild(head);  
	if (child != NULL)
	  visit_descendants(feat_count, ancestor, nsofar, f, child);
      }
    }
  }  // WSHeads::visit_descendants()

  // Here is the stuff that every feature needs

  //! The identifier string is WSHeads:nheads:governorlex:dependentlex:headtype.
  //
  virtual const char *identifier() const {
    return identifier_string.c_str();
  }  // WSHeads::identifier()

  SPFEATURES_COMMON_DEFINITIONS;
};  // WSHeads{}

//! The PTsFeatureClass is an ABC for feature classes where the feature
//! count for a tree is the sum of feature counts for each node, and the
//! feature counts for each node depends on the local tree and its ancestors,
//! and the node's left and right string positions.
//!
//! Every subclass of PTsFeatureClass must define a method:
//!
//!  node_featurecount(fc, preterminals, tp, feat_count)
//
class PTsFeatureClass : public TreeFeatureClass {
public:

  typedef std::vector<const sptree*> SptreePtrs;

  template <typename FeatClass, typename Feat_Count>
  static void tree_featurecount(FeatClass& fc, const sptree* tp, 
				Feat_Count& feat_count) {
    assert(tp != NULL);
    SptreePtrs preterms;
    tp->preterminal_nodes(preterms, true);
    if (preterms.size() != tp->label.right) {
      std::cerr << "## preterms = " << preterms 
		<< "\n## tp = " << tp << std::endl;
      return;
    }
    assert(preterms.size() == tp->label.right);
    tree_featurecount(fc, preterms, tp, feat_count);
  }  // PTsFeatureClass:tree_featurecount()

  //! tree_featurecount() actually visits the nodes
  //
  template <typename FeatClass, typename Feat_Count>
  static void tree_featurecount(FeatClass& fc, const SptreePtrs& preterms,
				const sptree* tp, Feat_Count& feat_count)
  {
    fc.node_featurecount(fc, preterms, tp, feat_count);
    if (tp->is_nonterminal())
      tree_featurecount(fc, preterms, tp->child, feat_count);
    if (tp->next)
      tree_featurecount(fc, preterms, tp->next, feat_count);
  }  // PTsFeatureClass:tree_featurecount()
  
};  // PTsFeatureClass{}


//! The Neighbours{} includes the node's category, its binned length
//! and the left and right POS tags next to each node.  This version
//! has a bug in it -- use Edges() to accomplish the same thing w/out
//! the bug!
//!
//! Its identifier is Neighbours:<nleft>:<nright>
//
class Neighbours : public PTsFeatureClass {
public:

  Neighbours(size_type nleft = 0,   //!< include nleft left POS
	     size_type nright = 0)  //!< include nright right POS
    : nleft(nleft), nright(nright), identifier_string("Neighbours:")
  {
    (identifier_string += lexical_cast<std::string>(nleft)) += ":";
    identifier_string += lexical_cast<std::string>(nright);
  }  // Neighbours::Neighbours()
    		   
  // required types

  typedef std::vector<symbol> Ss;
  typedef std::pair<int,Ss> ISs;

  typedef ISs Feature;

  size_type nleft, nright;           // number of POS tags to left and right
  std::string identifier_string;  // will hold its identifier

  template <typename FeatClass, typename Feat_Count>
  void node_featurecount(FeatClass& fc, const SptreePtrs& preterms,
			 const sptree* node, Feat_Count& feat_count)
  {
    if (!node->is_nonterminal())
      return;

    size_type left = node->label.left;
    size_type right = node->label.right;

    Feature f;
    // number of preterminals
    f.first = quantize(right-left); 
    f.second.push_back(node->label.cat);    // category label

    // XXXX The following line is buggy, but is left in here because this
    // XXXX is what was used in the ACL'05 and CoNLL'05 talks
    //
    for (size_type i = 0; i < nleft; ++i)      // left preterminals
      f.second.push_back(i <= left 
			 ? preterms[left-i]->label.cat : endmarker());

    for (size_type i = 0; i < nright; ++i)	    // right preterminals
      f.second.push_back(right+i < preterms.size() 
			 ? preterms[right+i]->label.cat : endmarker());

    ++feat_count[f];
  }  // Neighbours::node_featurecount()
 
  virtual const char *identifier() const {
    return identifier_string.c_str();
  }  // Neighbours::identifier()

  SPFEATURES_COMMON_DEFINITIONS;
};  // Neighbours{}


//! The WSEdges{} feature includes the preterminals, annotations and words surrounding
//! nonterminal categories.
//!
//! Its identifier is WSEdges:binnedlength: edge flags
//
class WSEdges : public PTsFeatureClass {
public:

  // required types

  typedef std::vector<symbol> Ss;
  typedef Ss Feature;

  struct E {
    int punct;     //!< number of punct to collect
    int pos;       //!< number of pos to collect
    int closed;    //!< number of closed class words to collect
    int word;      //!< number of words to collect
    int nsuffix;   //!< number of characters to keep from each word

    E(int punct = 0, int pos = 0, int closed = 0, int word = 0, int nsuffix=0) :
      punct(punct), pos(pos), closed(closed), word(word), nsuffix(nsuffix) { }

    std::string identifier() const {
      return lexical_cast<std::string>(punct) + ":" + lexical_cast<std::string>(pos) 
	+ ":" + lexical_cast<std::string>(closed) + ":" + lexical_cast<std::string>(word)
	+ ":" + lexical_cast<std::string>(nsuffix);
    }

    int width() const { return std::max(std::max(punct, pos), word); }

    void push_features(const SptreePtrs& preterms, 
		       int position,
		       int direction,
		       Feature& f) const {
      int n = preterms.size();

      for (int i = 0; i < punct; ++i) {
	int j = position+i*direction;
	f.push_back((j < 0 || j >= n) ? endmarker() 
		    : (preterms[j]->is_punctuation() ? preterms[j]->label.cat : ZERO() ));
      }

      for (int i = 0; i < pos; ++i) {
	int j = position+i*direction;
	f.push_back((j < 0 || j >= n) ? endmarker() :  preterms[j]->label.cat);
      }

      for (int i = 0; i < closed; ++i) {
	int j = position+i*direction;
	f.push_back((j < 0 || j >= n) ? endmarker() 
		    : (( preterms[j]->is_closed_class() || preterms[j]->is_punctuation() ) 
		       ? preterms[j]->child->label.cat : preterms[j]->label.cat ));
      }

      for (int i = 0; i < word; ++i) {
	int j = position+i*direction;
	f.push_back((j < 0 || j >= n) ? endmarker() : suffix(preterms[j]->child->label.cat, nsuffix));
      }

    }  // WSEdges::E:push_features()

  };  // WSEdges::E{}

  WSEdges(const E& leftleft,           //!< left side of constituent's left egde
	  const E& leftright,          //!< right side of constituent's left egde
	  const E& rightleft,          //!< left side of constituent's right egde
	  const E& rightright,         //!< right side of constituent's right egde
	  bool binned_length = false)  //!< include binned length
    : leftleft(leftleft), leftright(leftright), rightleft(rightleft), rightright(rightright),
      binned_length(binned_length), identifier_string("WSEdges:")
  {
    (identifier_string += lexical_cast<std::string>(binned_length)) += ":";
    ((identifier_string += "ll") += leftleft.identifier()) += ":";
    ((identifier_string += "lr") += leftright.identifier()) += ":";
    ((identifier_string += "rl") += rightleft.identifier()) += ":";
    ((identifier_string += "rr") += rightright.identifier());
  }  // WSEdges::WSEdges()
    		   
  const E leftleft, leftright, rightleft, rightright;
  bool binned_length;             // collect binned length
  std::string identifier_string;  // will hold its identifier

  template <typename FeatClass, typename Feat_Count>
  void node_featurecount(FeatClass& fc, const SptreePtrs& preterms,
			 const sptree* node, Feat_Count& feat_count)
  {
    if (!node->is_nonterminal())
      return;

    int left = node->label.left;
    int right = node->label.right;
    int nwords = preterms.size();

    // don't permit feature to overlap both edges
    //
    if (left + leftright.width() > right || left + rightleft.width() > right)
      return;

    if (left + 1 < leftleft.width())
      return;

    if (right + rightright.width() > nwords)
      return;

    Feature f;

    f.push_back(node->label.cat);               // category label

    if (binned_length)
      f.push_back(symbol_quantize(right-left)); // number of preterminals

    leftleft.push_features(preterms, left-1, -1, f);
    leftright.push_features(preterms, left, 1, f);
    rightleft.push_features(preterms, right-1, -1, f);
    rightright.push_features(preterms, right, 1, f);
    
    ++feat_count[f];
  }  // WSEdges::node_featurecount()
 
  virtual const char *identifier() const {
    return identifier_string.c_str();
  }  // WSEdges::identifier()

  SPFEATURES_COMMON_DEFINITIONS;
};  // WSEdges{}

//! The WEdges{} includes the node's category, its binned length
//! and the left and right POS and words preceding and following the constituent edges
//!
//! Its identifier is WEdges:<binnedlength>:<nleftprec>:<nleftprecw>:<nleftsucc>:<nleftsuccw>:<nrightprec>:<nrightprecw>:<nrightsucc>:<nrightsuccw>
//
class WEdges : public PTsFeatureClass {
public:

  WEdges(bool binned_length = false,  //!< include binned length
	 size_type nleftprec = 0,     //!< include nleft left preceding POS
	 size_type nleftprecw = 0,    //!< include nleft left preceding words
	 size_type nleftsucc = 0,     //!< include nleft left following POS
	 size_type nleftsuccw = 0,    //!< include nleft left following words
	 size_type nrightprec = 0,    //!< include nright right preceding POS
	 size_type nrightprecw = 0,   //!< include nright right preceding words
	 size_type nrightsucc = 0,    //!< include nright right following POS
	 size_type nrightsuccw = 0)   //!< include nright right following words
    : binned_length(binned_length), 
      nleftprec(nleftprec), nleftsucc(nleftsucc), nrightprec(nrightprec), nrightsucc(nrightsucc), 
      nleftprecw(nleftprecw), nleftsuccw(nleftsuccw), nrightprecw(nrightprecw), nrightsuccw(nrightsuccw), 
      identifier_string("WEdges:")
  {
    (identifier_string += lexical_cast<std::string>(binned_length)) += ":";
    (identifier_string += lexical_cast<std::string>(nleftprec)) += ":";
    (identifier_string += lexical_cast<std::string>(nleftprecw)) += ":";
    (identifier_string += lexical_cast<std::string>(nleftsucc)) += ":";
    (identifier_string += lexical_cast<std::string>(nleftsuccw)) += ":";
    (identifier_string += lexical_cast<std::string>(nrightprec)) += ":";
    (identifier_string += lexical_cast<std::string>(nrightprecw)) += ":";
    (identifier_string += lexical_cast<std::string>(nrightsucc)) += ":";
    identifier_string += lexical_cast<std::string>(nrightsuccw);
  }  // WEdges:W:Edges()
    		   
  // required types

  typedef std::vector<symbol> Ss;
  typedef Ss Feature;

  bool binned_length;             // collect binned length
  size_type nleftprec, nleftsucc, nrightprec, nrightsucc;  // number of words surrounding edges
  size_type nleftprecw, nleftsuccw, nrightprecw, nrightsuccw;  // number of words surrounding edges
  std::string identifier_string;  // will hold its identifier

  template <typename FeatClass, typename Feat_Count>
  void node_featurecount(FeatClass& fc, const SptreePtrs& preterms,
			 const sptree* node, Feat_Count& feat_count)
  {
    if (!node->is_nonterminal())
      return;

    size_type left = node->label.left;
    size_type right = node->label.right;
    size_type nwords = preterms.size();

    Feature f;
    if (binned_length)
      f.push_back(symbol_quantize(right-left)); // number of preterminals
    f.push_back(node->label.cat);               // category label

    for (size_type i = 1; i <= nleftprec; ++i)
      f.push_back(i <= left
		  ? preterms[left-i]->label.cat : endmarker());

    for (size_type i = 1; i <= nleftprecw; ++i)
      f.push_back(i <= left
		  ? preterms[left-i]->child->label.cat : endmarker());

    for (size_type i = 0; i < nleftsucc; ++i)
      f.push_back(left+i < nwords 
		  ? preterms[left+i]->label.cat : endmarker());
    
    for (size_type i = 0; i < nleftsuccw; ++i)
      f.push_back(left+i < nwords 
		  ? preterms[left+i]->child->label.cat : endmarker());
    
    for (size_type i = 1; i <= nrightprec; ++i)
      f.push_back(i <= right
		  ? preterms[right-i]->label.cat : endmarker());

    for (size_type i = 1; i <= nrightprecw; ++i)
      f.push_back(i <= right
		  ? preterms[right-i]->child->label.cat : endmarker());

    for (size_type i = 0; i < nrightsucc; ++i)	    // right preterminals
      f.push_back(right+i < nwords 
		  ? preterms[right+i]->label.cat : endmarker());

    for (size_type i = 0; i < nrightsuccw; ++i)	    // right words
      f.push_back(right+i < nwords 
		  ? preterms[right+i]->child->label.cat : endmarker());

    ++feat_count[f];
  }  // WEdges::node_featurecount()
 
  virtual const char *identifier() const {
    return identifier_string.c_str();
  }  // WEdges::identifier()

  SPFEATURES_COMMON_DEFINITIONS;
};  // WEdges{}

//! The Edges{} includes the node's category, its binned length
//! and the left and right POS preceding and following the constituent edges
//!
//! Its identifier is Edges:<binnedlength>:<nleftprec>:<nleftsucc>:<nrightprec>:<nrightsucc>
//
class Edges : public PTsFeatureClass {
public:

  Edges(bool binned_length = false,  //!< include binned length
	size_type nleftprec = 0,     //!< include nleft left preceding words
	size_type nleftsucc = 0,     //!< include nleft left following words
	size_type nrightprec = 0,    //!< include nright right preceding words
	size_type nrightsucc = 0)    //!< include nright right following words
    : binned_length(binned_length), nleftprec(nleftprec), nleftsucc(nleftsucc),
      nrightprec(nrightprec), nrightsucc(nrightsucc), identifier_string("Edges:")
  {
    (identifier_string += lexical_cast<std::string>(binned_length)) += ":";
    (identifier_string += lexical_cast<std::string>(nleftprec)) += ":";
    (identifier_string += lexical_cast<std::string>(nleftsucc)) += ":";
    (identifier_string += lexical_cast<std::string>(nrightprec)) += ":";
    identifier_string += lexical_cast<std::string>(nrightsucc);
  }  // Edges::Edges()
    		   
  // required types

  typedef std::vector<symbol> Ss;
  typedef Ss Feature;

  bool binned_length;             // collect binned length
  size_type nleftprec, nleftsucc, nrightprec, nrightsucc;  // number of words surrounding edges
  std::string identifier_string;  // will hold its identifier

  template <typename FeatClass, typename Feat_Count>
  void node_featurecount(FeatClass& fc, const SptreePtrs& preterms,
			 const sptree* node, Feat_Count& feat_count)
  {
    if (!node->is_nonterminal())
      return;

    size_type left = node->label.left;
    size_type right = node->label.right;
    size_type nwords = preterms.size();

    Feature f;
    if (binned_length)
      f.push_back(symbol_quantize(right-left)); // number of preterminals
    f.push_back(node->label.cat);               // category label

    for (size_type i = 1; i <= nleftprec; ++i)
      f.push_back(i <= left
		  ? preterms[left-i]->label.cat : endmarker());

    for (size_type i = 0; i < nleftsucc; ++i)
      f.push_back(left+i < nwords 
		  ? preterms[left+i]->label.cat : endmarker());
    
    for (size_type i = 1; i <= nrightprec; ++i)
      f.push_back(i <= right
		  ? preterms[right-i]->label.cat : endmarker());

    for (size_type i = 0; i < nrightsucc; ++i)	    // right preterminals
      f.push_back(right+i < nwords 
		  ? preterms[right+i]->label.cat : endmarker());

    ++feat_count[f];
  }  // Edges::node_featurecount()
 
  virtual const char *identifier() const {
    return identifier_string.c_str();
  }  // Edges::identifier()

  SPFEATURES_COMMON_DEFINITIONS;
};  // Edges{}

//! The WordNeighbours{} includes the node's category, its binned length
//! and the left and right words next to each node.
//!
//! Its identifier is WordNeighbours:<binnedlengthflag>:<nleft>:<nright>
//
class WordNeighbours : public PTsFeatureClass {
public:

  WordNeighbours(bool binned_length = false,  //!< include binned length
		 size_type nleft = 0,         //!< include nleft left words
		 size_type nright = 0)        //!< include nright right words
    : binned_length(binned_length), nleft(nleft), nright(nright), 
    identifier_string("WordNeighbours:")
  {
    (identifier_string += lexical_cast<std::string>(binned_length)) += ":";
    (identifier_string += lexical_cast<std::string>(nleft)) += ":";
    identifier_string += lexical_cast<std::string>(nright);
  }  // WordNeighbours::WordNeighbours()
    		   
  // required types

  typedef std::vector<symbol> Ss;
  typedef Ss Feature;

  bool binned_length;             // collect binned length
  size_type nleft, nright;        // number of words to left and right
  std::string identifier_string;  // will hold its identifier

  template <typename FeatClass, typename Feat_Count>
  void node_featurecount(FeatClass& fc, const SptreePtrs& preterms,
			 const sptree* node, Feat_Count& feat_count)
  {
    if (!node->is_nonterminal())
      return;

    size_type left = node->label.left;
    size_type right = node->label.right;

    Feature f;
    if (binned_length)
      f.push_back(symbol_quantize(right-left)); // number of preterminals
    f.push_back(node->label.cat);               // category label

    //  XXXXX BUG XXXXX  -- should be
    // for (size_type i = 1; i <= nleft; ++i)
    for (size_type i = 0; i < nleft; ++i)       // left preterminals
      f.push_back(i <= left 
		  ? preterms[left-i]->child->label.cat : endmarker());

    for (size_type i = 0; i < nright; ++i)	    // right preterminals
      f.push_back(right+i < preterms.size() 
		  ? preterms[right+i]->child->label.cat : endmarker());

    ++feat_count[f];
  }  // WordNeighbours::node_featurecount()
 
  virtual const char *identifier() const {
    return identifier_string.c_str();
  }  // WordNeighbours::identifier()

  SPFEATURES_COMMON_DEFINITIONS;
};  // WordNeighbours{}


//! The WordEdges{} includes the node's category, its binned length
//! and the left and right words preceding and following the constituent edges
//!
//! Its identifier is WordEdges:<binnedlength>:<nleftprec>:<nleftsucc>:<nrightprec>:<nrightsucc>
//
class WordEdges : public PTsFeatureClass {
public:

  WordEdges(bool binned_length = false,  //!< include binned length
	    size_type nleftprec = 0,     //!< include nleft left preceding words
	    size_type nleftsucc = 0,     //!< include nleft left following words
	    size_type nrightprec = 0,    //!< include nright right preceding words
	    size_type nrightsucc = 0)    //!< include nright right following words
    : binned_length(binned_length), nleftprec(nleftprec), nleftsucc(nleftsucc),
      nrightprec(nrightprec), nrightsucc(nrightsucc), identifier_string("WordEdges:")
  {
    (identifier_string += lexical_cast<std::string>(binned_length)) += ":";
    (identifier_string += lexical_cast<std::string>(nleftprec)) += ":";
    (identifier_string += lexical_cast<std::string>(nleftsucc)) += ":";
    (identifier_string += lexical_cast<std::string>(nrightprec)) += ":";
    identifier_string += lexical_cast<std::string>(nrightsucc);
  }  // WordEdges::WordEdges()
    		   
  // required types

  typedef std::vector<symbol> Ss;
  typedef Ss Feature;

  bool binned_length;             // collect binned length
  size_type nleftprec, nleftsucc, nrightprec, nrightsucc;  // number of words surrounding edges
  std::string identifier_string;  // will hold its identifier

  template <typename FeatClass, typename Feat_Count>
  void node_featurecount(FeatClass& fc, const SptreePtrs& preterms,
			 const sptree* node, Feat_Count& feat_count)
  {
    if (!node->is_nonterminal())
      return;

    size_type left = node->label.left;
    size_type right = node->label.right;
    size_type nwords = preterms.size();

    Feature f;
    if (binned_length)
      f.push_back(symbol_quantize(right-left)); // number of preterminals
    f.push_back(node->label.cat);               // category label

    for (size_type i = 1; i <= nleftprec; ++i)
      f.push_back(i <= left
		  ? preterms[left-i]->child->label.cat : endmarker());

    for (size_type i = 0; i < nleftsucc; ++i)
      f.push_back(left+i < nwords 
		  ? preterms[left+i]->child->label.cat : endmarker());
    
    for (size_type i = 1; i <= nrightprec; ++i)
      f.push_back(i <= right
		  ? preterms[right-i]->child->label.cat : endmarker());

    for (size_type i = 0; i < nrightsucc; ++i)	    // right preterminals
      f.push_back(right+i < nwords 
		  ? preterms[right+i]->child->label.cat : endmarker());

    ++feat_count[f];
  }  // WordEdges::node_featurecount()
 
  virtual const char *identifier() const {
    return identifier_string.c_str();
  }  // WordEdges::identifier()

  SPFEATURES_COMMON_DEFINITIONS;
};  // WordEdges{}


//! The Heavy{} classifies nodes by their size and 
//! how close to the end of the sentence they occur, as well as whether
//! they are followed by punctuation or coordination.
//
class Heavy : public PTsFeatureClass {
public:

  typedef std::vector<symbol> Ss;
  typedef std::vector<int> Is;
  typedef std::pair<Is,Ss> Feature;

  template <typename FeatClass, typename Feat_Count>
  void node_featurecount(FeatClass& fc, const SptreePtrs& preterms,
			 const sptree* node, Feat_Count& feat_count)
  {
    if (!node->is_nonterminal())
      return;

    symbol final_punct = endmarker();
    symbol following_punct = endmarker();

    assert(node->label.right > 0);
    assert(node->label.right <= preterms.size());

    if (preterms[node->label.right-1]->is_punctuation())
      final_punct = preterms[node->label.right-1]->child->label.cat;

    if (node->label.right < preterms.size() 
	&& preterms[node->label.right]->is_punctuation())
      following_punct = preterms[node->label.right]->child->label.cat;

    Feature f;
    // number of preterminals
    f.first.push_back(quantize(node->label.right-node->label.left));
    f.first.push_back(quantize(preterms.size()-node->label.right));      // how far from end
    f.second.push_back(node->label.cat);     // category label
    f.second.push_back(final_punct);
    f.second.push_back(following_punct);

    ++feat_count[f];
  }  // Heavy::node_featurecount()

  virtual const char *identifier() const {
    return "Heavy";
  }  // Heavy::identifier()

  SPFEATURES_COMMON_DEFINITIONS;
};  // Heavy{}


//! NGramTree identifies n-gram tree fragments.  The identifier is
//!
//! NGramTree:ngram:lexicalize:collapse:nancs
//
class NGramTree : public TreeFeatureClass {
public:

  // required types

  typedef sstring Feature;

  enum lexicalize_type { none, closed_class, functional, all };

  int ngram;                      //!< # of words in context
  lexicalize_type lexicalize;     //!< lexicalize preterminals
  bool collapse;                  //!< collapse nodes not dominating ngram
  int nancs;                      //!< extra ancestors to go up
  std::string identifier_string;  //!< returned by identifier()

  NGramTree(int ngram = 2, lexicalize_type lexicalize = none, bool collapse = false,
	    int nancs = 0) 
    : ngram(ngram), lexicalize(lexicalize), collapse(collapse), nancs(nancs),
      identifier_string("NGramTree:") 
  { 
    identifier_string += lexical_cast<std::string>(ngram) + ":";
    identifier_string += lexical_cast<std::string>(lexicalize) + ":";
    identifier_string += lexical_cast<std::string>(collapse) + ":";
    identifier_string += lexical_cast<std::string>(nancs);
  }  // NGramTree::NGramTree()

  tree* selective_copy(const sptree* sp, size_type left, size_type right, 
		       bool copy_next = false) 
  {
    const sptree_label& label = sp->label;

    if (collapse) {
      if (label.right <= left) 
	return (sp->next && copy_next) 
	  ? selective_copy(sp->next, left, right, copy_next) : NULL;
      else if (label.left >= right)
	return NULL;
    }

    tree* t = new tree(label);
    if (sp->child && label.left < right && label.right > left
	&& (sp->is_nonterminal()
	    || lexicalize == all
	    || (lexicalize == functional && sp->is_functional())
	    || (lexicalize == closed_class && sp->is_closed_class())))
      t->child = selective_copy(sp->child, left, right, true);

    if (copy_next && sp->next)
      t->next = selective_copy(sp->next, left, right, copy_next);

    return t;
  }  // NGramTree::selective_copy()

  template <typename FeatClass, typename Feat_Count>
  void tree_featurecount(FeatClass& fc, const sptree* root, 
			 Feat_Count& feat_count) {
    if (debug_level >= 10000)
      std::cerr << "# root = " << root << std::endl;
    std::vector<const sptree*> preterms;
    root->preterminal_nodes(preterms);
    for (size_type i = 0; i + ngram < preterms.size(); ++i) {
      const sptree* t0;
      for (t0 = preterms[i]; t0 != NULL && t0->label.right < i + ngram; 
	   t0 = t0->label.parent)
	;

      assert(t0 != NULL);

      for (int ianc = 0; ianc < nancs && t0; ++ianc)
	t0 = t0->label.parent;   // go up extra ancestors

      if (t0 == NULL)
	return;

      tree* frag = selective_copy(t0, i, i + ngram);
      Feature feat(frag);
      if (debug_level >= 20000)
	std::cerr << "#  " << preterms[i]->child->label.cat 
		  << ": " << feat << std::endl;
      ++feat_count[feat];
      delete frag;
    }
  }  // NGramTree::tree_featurecount()
 
  virtual const char *identifier() const {
    return identifier_string.c_str();
  }  // NGramTree::identifier()

  SPFEATURES_COMMON_DEFINITIONS;
};  // NGramTree{}


//! HeadTree
//!
//! Identifier is HeadTree:collapse:lexicalize:nancs:headtype
//
class HeadTree : public NodeFeatureClass {
public:

  // required types

  typedef sstring Feature;

  enum head_type { syntactic, semantic };

  bool collapse;                 //!< collapse nodes not dominating ngram
  bool lexicalize;		 //!< include lexical item
  int nancs;                     //!< extra ancestors to go up
  head_type htype;               //!< type of heads to project
  std::string identifier_string; //!< returned by identifier()

  HeadTree(bool collapse = true, bool lexicalize = false, 
	   int nancs = 0, head_type htype = semantic) 
    : collapse(collapse), lexicalize(lexicalize), nancs(nancs), htype(htype),
      identifier_string("HeadTree:") 
  { 
    identifier_string += lexical_cast<std::string>(collapse) + ":";
    identifier_string += lexical_cast<std::string>(lexicalize) + ":";
    identifier_string += lexical_cast<std::string>(nancs) + ":";
    identifier_string += lexical_cast<std::string>(htype);
  }  // HeadTree::HeadTree()

  tree* selective_copy(const sptree* sp, unsigned int headleft) 
  {
    if (!sp)
      return NULL;

    const sptree_label& label = sp->label;

    if (collapse) {
      unsigned int left = label.previous ? label.previous->label.left : label.left;
      unsigned int right = sp->next ? sp->next->label.right : label.right;
      if (right <= headleft) 
	return selective_copy(sp->next, headleft);
      else if (left > headleft)
	return NULL;
    }

    return new tree(label,
		    (sp->is_nonterminal() || (lexicalize && label.left == headleft)) 
		    ? selective_copy(sp->child, headleft) : NULL,
		    selective_copy(sp->next, headleft));
  }  // HeadTree::selective_copy()

  template <typename FeatClass, typename Feat_Count>
  void tree_featurecount(FeatClass& fc, const sptree* root, 
			 Feat_Count& feat_count) {
    if (debug_level >= 20000)
      std::cerr << "# root = " << root << std::endl;
    std::vector<const sptree*> preterms;
    root->preterminal_nodes(preterms);
    for (size_type i = 0; i < preterms.size(); ++i) {
      const sptree* t0 = preterms[i];
      while (true) {
	const sptree* parent = t0->label.parent;
	if (!parent)
	  break;
	const sptree* hchild  = (htype == syntactic) 
	  ? parent->label.syntactic_headchild
	  : parent->label.semantic_headchild;
	if (hchild != t0)
	  break;
	t0 = parent;
      }

      assert(t0 != NULL);

      for (int ianc = 0; ianc < nancs && t0; ++ianc)
	t0 = t0->label.parent;   // go up extra ancestors

      if (t0 == NULL)
	return;

      tree* frag = selective_copy(t0, i);
      Feature feat(frag);
      if (debug_level >= 20000)
	std::cerr << "#  " << preterms[i]->child->label.cat 
		  << ": " << feat << std::endl;
      ++feat_count[feat];
      delete frag;
    }
  }  // HeadTree::tree_featurecount()
 
  virtual const char *identifier() const {
    return identifier_string.c_str();
  }  // HeadTree::identifier()

  SPFEATURES_COMMON_DEFINITIONS;
};  // HeadTree{}


class SubjVerbAgr : public NodeFeatureClass {
public:

  typedef std::vector<symbol> Feature;

  template <typename FeatClass, typename Feat_Count>
  void node_featurecount(FeatClass& fc, const sptree* node, 
			 Feat_Count& feat_count) 
  {
    if ((node->label.cat != S() && node->label.cat != SINV()) 
	|| node->label.syntactic_lexhead == NULL)
      return;
    
    const sptree* subject = NULL;

    // subject is last NP before VP

    for (const sptree* child = node->child; child != NULL; child = child->next) 
      if (child->label.cat == NP())
	subject = child;
      else if (child->label.cat == VP())
	break;

    if (subject == NULL || subject->label.semantic_lexhead == NULL)
      return;
    
    Feature f;
    f.push_back(subject->label.semantic_lexhead->label.cat);
    f.push_back(node->label.syntactic_lexhead->label.cat);
    ++feat_count[f];
  }  // SubjVerbAgr::node_featurecount()

  virtual const char *identifier() const {
    return "SubjVerbAgr";
  }  // SubjVerbAgr::identifier()

  SPFEATURES_COMMON_DEFINITIONS;
};  // SubjVerbAgr{}

class SynSemHeads : public NodeFeatureClass {
public:

  typedef std::vector<symbol> Feature;

  enum annotation_type { none, lex_syn, lex_all };
  annotation_type ann;
  std::string identifier_string; //!< identifier string, returned by identifier()
  
  SynSemHeads(annotation_type ann = none) 
    : ann(ann), identifier_string("SynSemHeads:")
  {
    identifier_string += lexical_cast<std::string>(ann);
  }  // SynSemHeads::SynSemHeads()

  template <typename FeatClass, typename Feat_Count>
  void node_featurecount(FeatClass& fc, const sptree* node, 
			 Feat_Count& feat_count) 
  {
    const sptree_label& label = node->label;

    if (label.syntactic_lexhead == label.semantic_lexhead)
      return;

    Feature f;
    f.push_back(label.syntactic_lexhead ? label.syntactic_lexhead->label.cat : endmarker());
    if (ann != none) {
      if (label.syntactic_lexhead == NULL)
	return;
      else
	f.push_back(label.syntactic_lexhead->child->label.cat);
    }

    f.push_back(label.semantic_lexhead ? label.semantic_lexhead->label.cat : endmarker());

    if (ann == lex_all) {
      if (label.semantic_lexhead == NULL)
	return;
      else
	f.push_back(label.semantic_lexhead->child->label.cat);
    }
    ++feat_count[f];
  }  // SynSemHeads::node_featurecount()

  virtual const char *identifier() const {
    return identifier_string.c_str();
  }  // SynSemHeads::identifier()

  SPFEATURES_COMMON_DEFINITIONS;
};  // SynSemHeads{}


//////////////////////////////////////////////////////////////////////////
//                                                                      //
//                        Coordination features                         //
//                                                                      //
//////////////////////////////////////////////////////////////////////////


//! CoPar{} counts the number of parallel and non-parallel coordinations
//! at various levels
//!
//! Identifier is CoPar:IgnorePreterms
//
class CoPar : public NodeFeatureClass {
  bool ignore_preterms;
  std::string identifier_string;

public:

  CoPar(bool ignore_preterms = false) 
    : ignore_preterms(ignore_preterms), identifier_string("CoPar:")
  {
    identifier_string += lexical_cast<std::string>(ignore_preterms);
  }  // CoPar::CoPar()


  template <typename FeatClass, typename Feat_Count>
  void node_featurecount(FeatClass& fc, const sptree* node, 
			 Feat_Count& feat_count) 
  {
    if (!node->is_coordination())
      return;
    for (int depth = 1; depth <= 5; ++depth) {
      const sptree* last_child = NULL;
      for (const sptree* child = node->child; child; child = child->next) {
	if (child->is_punctuation() || child->is_conjunction())
	  continue;
	if (last_child != NULL) {
	  int m = match(depth, last_child, child);
	  if (m != -1)
	    ++feat_count[Feature(depth,m)];
	}
	last_child = child;
      }
    }
  }  // CoPar::node_feature_count()


  //! match() returns 1 if node1 and node2 match to depth, 0 if they mismatch
  //! and -1 if they match but do not have any subnodes at depth.
  //
  int match(int depth, const sptree* node1, const sptree* node2) const {
    assert(node1 != NULL);
    assert(node2 != NULL);
    if (node1->label.cat != node2->label.cat)
      return 0;
    if (depth == 1)
      return 1;
    if (node1->is_preterminal()) {
      assert(node2->is_preterminal());
      return -1;
    }
    return matches(depth-1, node1->child, node2->child);
  }

  //! matches() is responsible for matching node1 and node2 and their right
  //! siblings
  //
  int matches(int depth, const sptree* node1, const sptree* node2) const {
    assert(depth >= 1);

    if (ignore_preterms) {
      while (node1 != NULL && node1->is_preterminal())
	node1 = node1->next;
      while (node2 != NULL && node2->is_preterminal())
	node2 = node2->next;
    }

    if (node1 == NULL) 
      return (node2 == NULL) ? -1 : 0;
    
    if (node2 == NULL)
      return 0;
    
    int m1 = match(depth, node1, node2);
    int m2 = matches(depth, node1->next, node2->next);
    
    if (m1 == 0 || m2 == 0)
      return 0;
    else if (m1 == 1 || m2 == 1)
      return 1;
    else
      return -1;
  }

  virtual const char* identifier() const {
    return identifier_string.c_str();
  }  // CoPar::identifier()

  typedef std::pair<int,int> Feature;

  SPFEATURES_COMMON_DEFINITIONS;
};


//! CoLenPar{} counts the number of adjacent conjuncts that have
//!  the same length, are shorter, and are longer.
//
class CoLenPar : public NodeFeatureClass {
public:

  template <typename FeatClass, typename Feat_Count>
  void node_featurecount(FeatClass& fc, const sptree* node, 
			 Feat_Count& feat_count) 
  {
    if (!node->is_coordination())
      return;
    const sptree* last_child = NULL;
    int last_size = 0;
    for (const sptree* child = node->child; child; child = child->next) {
      if (child->is_punctuation() || child->is_conjunction())
	continue;
      int size = child->label.right - child->label.left;
      if (last_child != NULL) {
	int dsize = size - last_size;
	if (dsize > 4)
	  dsize = 5;
	else if (dsize < -4)
	  dsize = -5;
	++feat_count[Feature(dsize,child->next==NULL)];
      }
      last_child = child;
      last_size = size;
    }
  }  // CoLenPar::node_feature_count()

  virtual const char* identifier() const {
    return "CoLenPar";
  }  // CoLenPar::identifier()

  typedef std::pair<int,int> Feature;

  SPFEATURES_COMMON_DEFINITIONS;
};

inline void FeatureClassPtrs::features_connll() {
  push_back(new NLogP());

  push_back(new Rule());
  push_back(new Rule(0, 1));
  push_back(new Rule(0, 0, true));
  push_back(new Rule(0, 0, false, true));
  push_back(new Rule(0, 0, false, false, Rule::lexical));
  push_back(new Rule(0, 0, false, false, Rule::none, Rule::lexical));
  push_back(new Rule(0, 0, false, false, Rule::lexical, Rule::lexical));
  push_back(new Rule(1, 0));
  push_back(new Rule(1, 1));

  push_back(new NGram(1, 1, false, true));
  push_back(new NGram(2, 1, true, true));
  push_back(new NGram(3, 1, true, true));
  push_back(new NGram(2, 1, false, false, NGram::lexical));
  push_back(new NGram(2, 1, false, false, NGram::none, NGram::lexical));

  push_back(new Word(1));
  push_back(new Word(2));

  push_back(new WProj());

  push_back(new RightBranch());

  push_back(new Heavy());

  push_back(new NGramTree(2, NGramTree::none, true));
  push_back(new NGramTree(2, NGramTree::all, true));
  push_back(new NGramTree(3, NGramTree::functional, true));

  push_back(new HeadTree(true, false, 0, HeadTree::syntactic));
  push_back(new HeadTree(true, false, 0, HeadTree::semantic));
  push_back(new HeadTree(true, true, 0, HeadTree::semantic));
  
  push_back(new Heads(2, false, false, Heads::syntactic));
  push_back(new Heads(2, true, true, Heads::syntactic));
  push_back(new Heads(2, true, true, Heads::semantic));
  push_back(new Heads(3, false, false));

  push_back(new Neighbours(0,0));
  push_back(new Neighbours(0,1));
  push_back(new Neighbours(1,0));

  push_back(new CoPar(false));

  push_back(new CoLenPar());

} // FeatureClassPtrs::features_connll()


inline void FeatureClassPtrs::features_050902() {
  push_back(new NLogP());

  push_back(new Rule());
  push_back(new Rule(0, 1));
  push_back(new Rule(0, 0, true));
  push_back(new Rule(0, 0, false, true));
  push_back(new Rule(0, 0, false, false, Rule::lexical));
  push_back(new Rule(0, 0, false, false, Rule::none, Rule::lexical));
  push_back(new Rule(0, 0, false, false, Rule::lexical, Rule::lexical));

  push_back(new NGram(1, 1, false, true));
  push_back(new NGram(2, 1, true, true));
  push_back(new NGram(3, 1, true, true));
  push_back(new NGram(2, 1, false, false, NGram::lexical));
  push_back(new NGram(2, 1, false, false, NGram::none, NGram::lexical));

  push_back(new Word(1));
  push_back(new Word(2));

  push_back(new WProj());

  push_back(new RightBranch());

  push_back(new Heavy());

  push_back(new NGramTree(2, NGramTree::none, true));
  push_back(new NGramTree(2, NGramTree::all, true));
  push_back(new NGramTree(3, NGramTree::functional, true));

  push_back(new HeadTree(true, false, 0, HeadTree::syntactic));
  push_back(new HeadTree(true, false, 0, HeadTree::semantic));
  push_back(new HeadTree(true, true, 0, HeadTree::semantic));
  
  push_back(new Heads(2, false, false, Heads::syntactic));
  push_back(new Heads(2, true, true, Heads::syntactic));
  push_back(new Heads(2, true, true, Heads::semantic));
  push_back(new Heads(3, false, false));

  push_back(new CoPar(false));

  push_back(new CoLenPar());

  size_type maxwidth = 2, maxsumwidth = 2;

  for (size_type binflag = 0; binflag < 2; ++binflag)
    for (size_type nleftprec = 0; nleftprec <= maxwidth; ++nleftprec)
      for (size_type nleftsucc = 0; nleftsucc <= maxwidth; ++nleftsucc)
	for (size_type nrightprec = 0; nrightprec <= maxwidth; ++nrightprec)
	  for (size_type nrightsucc = 0; nrightsucc <= maxwidth; ++nrightsucc)
	    if (nleftprec + nleftsucc + nrightprec + nrightsucc <= maxsumwidth)
	      push_back(new Edges(binflag, nleftprec, nleftsucc, nrightprec, nrightsucc));

  for (size_type binflag = 0; binflag < 2; ++binflag)
    for (size_type nleftprec = 0; nleftprec <= maxwidth; ++nleftprec)
      for (size_type nleftsucc = 0; nleftsucc <= maxwidth; ++nleftsucc)
	for (size_type nrightprec = 0; nrightprec <= maxwidth; ++nrightprec)
	  for (size_type nrightsucc = 0; nrightsucc <= maxwidth; ++nrightsucc)
	    if (nleftprec + nleftsucc + nrightprec + nrightsucc <= maxsumwidth)
	      push_back(new WordEdges(binflag, nleftprec, nleftsucc, nrightprec, nrightsucc));

}  // FeatureClassPtrs::features_050902()


inline void FeatureClassPtrs::features_071114() {
  push_back(new NLogP());

  push_back(new RBContext(false, false, false));
  push_back(new RBContext(false, true, false));
  push_back(new RBContext(false, true, true));
  push_back(new RBContext(true, false, false));
  push_back(new RBContext(true, true, false));
  push_back(new RBContext(true, true, true));

  push_back(new Rule(false, false, false, RBContext::syntactic));
  push_back(new Rule(true, false, false, RBContext::syntactic));
  push_back(new Rule(true, true, true, RBContext::syntactic));

  push_back(new Rule(0, 1));
  push_back(new Rule(0, 0, true));
  push_back(new Rule(0, 0, false, true));
  push_back(new Rule(0, 0, false, false, Rule::lexical));
  push_back(new Rule(0, 0, false, false, Rule::none, Rule::lexical));
  push_back(new Rule(0, 0, false, false, Rule::lexical, Rule::lexical));

  push_back(new NGram(1, 1, false, true));
  push_back(new NGram(2, 1, true, true));
  push_back(new NGram(3, 1, true, true));
  push_back(new NGram(2, 1, false, false, NGram::lexical));
  push_back(new NGram(2, 1, false, false, NGram::none, NGram::lexical));

  push_back(new Word(1));
  push_back(new Word(2));

  push_back(new WProj());

  push_back(new RightBranch());

  push_back(new Heavy());

  push_back(new NGramTree(2, NGramTree::none, true));
  push_back(new NGramTree(2, NGramTree::all, true));
  push_back(new NGramTree(3, NGramTree::functional, true));

  push_back(new HeadTree(true, false, 0, HeadTree::syntactic));
  push_back(new HeadTree(true, false, 0, HeadTree::semantic));
  push_back(new HeadTree(true, true, 0, HeadTree::semantic));
  
  push_back(new Heads(2, false, false, Heads::syntactic));
  push_back(new Heads(2, true, true, Heads::syntactic));
  push_back(new Heads(2, true, true, Heads::semantic));
  push_back(new Heads(3, false, false));

  push_back(new CoPar(false));

  push_back(new CoLenPar());

  size_type maxwidth = 2, maxsumwidth = 2;

  for (size_type binflag = 0; binflag < 2; ++binflag)
    for (size_type nleftprec = 0; nleftprec <= maxwidth; ++nleftprec)
      for (size_type nleftsucc = 0; nleftsucc <= maxwidth; ++nleftsucc)
	for (size_type nrightprec = 0; nrightprec <= maxwidth; ++nrightprec)
	  for (size_type nrightsucc = 0; nrightsucc <= maxwidth; ++nrightsucc)
	    if (nleftprec + nleftsucc + nrightprec + nrightsucc <= maxsumwidth)
	      push_back(new Edges(binflag, nleftprec, nleftsucc, nrightprec, nrightsucc));

  for (size_type binflag = 0; binflag < 2; ++binflag)
    for (size_type nleftprec = 0; nleftprec <= maxwidth; ++nleftprec)
      for (size_type nleftsucc = 0; nleftsucc <= maxwidth; ++nleftsucc)
	for (size_type nrightprec = 0; nrightprec <= maxwidth; ++nrightprec)
	  for (size_type nrightsucc = 0; nrightsucc <= maxwidth; ++nrightsucc)
	    if (nleftprec + nleftsucc + nrightprec + nrightsucc <= maxsumwidth)
	      push_back(new WordEdges(binflag, nleftprec, nleftsucc, nrightprec, nrightsucc));

}  // FeatureClassPtrs::features_071114()


//! features_splh() are a revision of features_050902() based on suggestions made
//! by Liang Huang. 
//
inline void FeatureClassPtrs::features_splh(bool local, bool nngram) {
  push_back(new NLogP());
  push_back(new RightBranch());
  push_back(new Heavy());

  push_back(new RBContext(false, false, false));
  push_back(new RBContext(false, true, false));
  push_back(new RBContext(false, true, true));
  push_back(new RBContext(true, false, false));
  push_back(new RBContext(true, true, false));
  push_back(new RBContext(true, true, true));

  push_back(new Rule(false, false, false, RBContext::syntactic));
  push_back(new Rule(true, false, false, RBContext::syntactic));
  push_back(new Rule(true, true, true, RBContext::syntactic));

  push_back(new Rule(0, 1));
  push_back(new Rule(0, 0, true));
  push_back(new Rule(0, 0, false, true));
  push_back(new Rule(0, 0, false, false, Rule::lexical));
  push_back(new Rule(0, 0, false, false, Rule::none, Rule::lexical));
  push_back(new Rule(0, 0, false, false, Rule::lexical, Rule::lexical));

  push_back(new NGram(1, 1, false, true));
  push_back(new NGram(2, 1, true, true));
  push_back(new NGram(3, 1, true, true));
  push_back(new NGram(2, 1, false, false, NGram::lexical));
  push_back(new NGram(2, 1, false, false, NGram::none, NGram::lexical));

  if (nngram) {
    push_back(new NNGram(1, 1, false, true, NNGram::none, NNGram::none, NNGram::none, NNGram::syntactic, true, true));
    push_back(new NNGram(2, 1, true, true, NNGram::none, NNGram::none, NNGram::none, NNGram::syntactic, true, true));
    push_back(new NNGram(3, 1, true, true, NNGram::none, NNGram::none, NNGram::none, NNGram::syntactic, true, true));
    push_back(new NNGram(2, 1, false, false, NNGram::lexical, NNGram::none, NNGram::none, NNGram::syntactic, true, true));
    push_back(new NNGram(2, 1, false, false, NNGram::lexical, NNGram::lexical, NNGram::none, NNGram::syntactic, true, true));
  }

  push_back(new Word(1));
  push_back(new Word(2));

  push_back(new WProj());

  push_back(new HeadTree(true, false, 0, HeadTree::syntactic));
  push_back(new HeadTree(true, false, 0, HeadTree::semantic));
  push_back(new HeadTree(true, true, 0, HeadTree::semantic));
  
  push_back(new Heads(2, false, false, Heads::syntactic));
  push_back(new Heads(2, true, true, Heads::syntactic));
  push_back(new Heads(2, true, true, Heads::semantic));
  push_back(new Heads(3, false, false));

  size_type maxwidth = 2, maxsumwidth = 3;

  for (size_type binflag = 0; binflag < 2; ++binflag)
    for (size_type nleftprec = 0; nleftprec <= maxwidth; ++nleftprec)
      for (size_type nleftsucc = 0; nleftsucc <= maxwidth; ++nleftsucc)
	for (size_type nrightprec = 0; nrightprec <= maxwidth; ++nrightprec)
	  for (size_type nrightsucc = 0; nrightsucc <= maxwidth; ++nrightsucc)
	    if (nleftprec + nleftsucc + nrightprec + nrightsucc <= maxsumwidth)
	      push_back(new Edges(binflag, nleftprec, nleftsucc, nrightprec, nrightsucc));

  for (size_type binflag = 0; binflag < 2; ++binflag)
    for (size_type nleftprec = 0; nleftprec <= maxwidth; ++nleftprec)
      for (size_type nleftsucc = 0; nleftsucc <= maxwidth; ++nleftsucc)
	for (size_type nrightprec = 0; nrightprec <= maxwidth; ++nrightprec)
	  for (size_type nrightsucc = 0; nrightsucc <= maxwidth; ++nrightsucc)
	    if (nleftprec + nleftsucc + nrightprec + nrightsucc <= maxsumwidth)
	      push_back(new WordEdges(binflag, nleftprec, nleftsucc, nrightprec, nrightsucc));

  if (!local) {
    push_back(new NGramTree(2, NGramTree::none, true));
    push_back(new NGramTree(2, NGramTree::all, true));
    push_back(new NGramTree(3, NGramTree::functional, true));

    push_back(new CoPar(false));
    push_back(new CoLenPar());
  }

}  // FeatureClassPtrs::features_splh()

//! features_splhsuffix() adds suffix features to features_splh()
//
inline void FeatureClassPtrs::features_splhsuffix(size_type nsuffix, bool local) {
  push_back(new NLogP());
  push_back(new RightBranch());
  push_back(new Heavy());

  push_back(new RBContext(false, false, false));
  push_back(new RBContext(false, true, false));
  push_back(new RBContext(false, true, true));
  push_back(new RBContext(true, false, false));
  push_back(new RBContext(true, true, false));
  push_back(new RBContext(true, true, true));

  push_back(new Rule(false, false, false, RBContext::syntactic));
  push_back(new Rule(true, false, false, RBContext::syntactic));
  push_back(new Rule(true, true, true, RBContext::syntactic));

  push_back(new Rule(0, 1));
  push_back(new Rule(0, 0, true));
  push_back(new Rule(0, 0, false, true));
  push_back(new Rule(0, 0, false, false, Rule::lexical));
  push_back(new Rule(0, 0, false, false, Rule::none, Rule::lexical));
  push_back(new Rule(0, 0, false, false, Rule::lexical, Rule::lexical));

  push_back(new NGram(1, 1, false, true));
  push_back(new NGram(2, 1, true, true));
  push_back(new NGram(3, 1, true, true));
  push_back(new NGram(2, 1, false, false, NGram::lexical));
  push_back(new NGram(2, 1, false, false, NGram::none, NGram::lexical));

  push_back(new Word(1));
  push_back(new Word(2));

  push_back(new WProj());

  push_back(new HeadTree(true, false, 0, HeadTree::syntactic));
  push_back(new HeadTree(true, false, 0, HeadTree::semantic));
  push_back(new HeadTree(true, true, 0, HeadTree::semantic));
  
  push_back(new WSHeads(0, true, 2, WSHeads::pos, WSHeads::pos, WSHeads::syntactic));
  push_back(new WSHeads(0, true, 2, WSHeads::lexical, WSHeads::lexical, WSHeads::syntactic));
  push_back(new WSHeads(0, true, 2, WSHeads::lexical, WSHeads::lexical, WSHeads::semantic));
  push_back(new WSHeads(0, true, 3, WSHeads::pos, WSHeads::pos, WSHeads::syntactic));
  if (nsuffix > 0) {
    push_back(new WSHeads(nsuffix, true, 2, WSHeads::lexical, WSHeads::lexical, WSHeads::syntactic));
    //  push_back(new WSHeads(nsuffix, true, 2, WSHeads::lexical, WSHeads::lexical, WSHeads::semantic));
  }

  size_type maxwidth = 2, maxsumwidth = 3;

  for (size_type binflag = 0; binflag < 2; ++binflag)
    for (size_type nleftprec = 0; nleftprec <= maxwidth; ++nleftprec)
      for (size_type nleftsucc = 0; nleftsucc <= maxwidth; ++nleftsucc)
	for (size_type nrightprec = 0; nrightprec <= maxwidth; ++nrightprec)
	  for (size_type nrightsucc = 0; nrightsucc <= maxwidth; ++nrightsucc)
	    if (nleftprec + nleftsucc + nrightprec + nrightsucc <= maxsumwidth) {
	      typedef WSEdges::E E;
	      push_back(new WSEdges(E(0,nleftprec,0,0,0), E(0,nleftsucc,0,0,0), E(0,nrightprec,0,0,0), E(0,nrightsucc,0,0,0), binflag));
	      if (nleftprec + nleftsucc + nrightprec + nrightsucc > 0) 
		push_back(new WSEdges(E(0,nleftprec,0,nleftprec,0), E(0,nleftsucc,0,nleftsucc,0), 
				      E(0,nrightprec,0,nrightprec,0), E(0,nrightsucc,0,nrightsucc,0), binflag));
	      if (nleftprec + nleftsucc + nrightprec + nrightsucc > 0 && nsuffix > 0) 
		push_back(new WSEdges(E(0,nleftprec,0,nleftprec,nsuffix), E(0,nleftsucc,0,nleftsucc,nsuffix), 
				      E(0,nrightprec,0,nrightprec,nsuffix), E(0,nrightsucc,0,nrightsucc,nsuffix), binflag));
	    }

  if (!local) {
    push_back(new NGramTree(2, NGramTree::none, true));
    push_back(new NGramTree(2, NGramTree::all, true));
    push_back(new NGramTree(3, NGramTree::functional, true));

    push_back(new CoPar(false));
    push_back(new CoLenPar());
  }

}  // FeatureClassPtrs::features_splhsuffix()

inline void FeatureClassPtrs::features_wedges() {
  push_back(new NLogP());

  push_back(new RightBranch());

  push_back(new Heavy());

  size_type maxwidth = 1, maxsumwidth = 2;

  for (size_type binflag = 0; binflag < 2; ++binflag)
    for (size_type nleftprec = 0; nleftprec <= maxwidth; ++nleftprec)
      for (size_type nleftsucc = 0; nleftsucc <= maxwidth; ++nleftsucc)
	for (size_type nrightprec = 0; nrightprec <= maxwidth; ++nrightprec)
	  for (size_type nrightsucc = 0; nrightsucc <= maxwidth; ++nrightsucc)
	    if (nleftprec + nleftsucc + nrightprec + nrightsucc <= maxsumwidth)
	      for (size_type nleftprecw = 0; nleftprecw <= nleftprec; ++nleftprecw)
		for (size_type nleftsuccw = 0; nleftsuccw <= nleftsucc; ++nleftsuccw)
		  for (size_type nrightprecw = 0; nrightprecw <= nrightprec; ++nrightprecw)
		    for (size_type nrightsuccw = 0; nrightsuccw <= nrightsucc; ++nrightsuccw)
		      push_back(new WEdges(binflag, nleftprec, nleftprecw, nleftsucc, nleftsuccw, nrightprec, nrightprecw, nrightsucc, nrightsuccw));

}  // FeatureClassPtrs::features_wedges()

//! wsfeatures() is a combination of head and edge features.  Last updated 20th July 2008
//
inline void FeatureClassPtrs::wsfeatures(bool headfeatures, int edgefeatures, bool ngram, bool ngramtree, bool rbcontext) {
  push_back(new NLogP());
  push_back(new RightBranch());
  push_back(new Heavy());

  if (headfeatures) {
    push_back(new WSHeads(0, true, 2, WSHeads::pos, WSHeads::pos, WSHeads::syntactic));
    push_back(new WSHeads(0, true, 2, WSHeads::pos, WSHeads::closedclass, WSHeads::syntactic));
    push_back(new WSHeads(0, true, 2, WSHeads::closedclass, WSHeads::pos, WSHeads::syntactic));
    push_back(new WSHeads(0, true, 2, WSHeads::closedclass, WSHeads::closedclass, WSHeads::syntactic));
    push_back(new WSHeads(0, true, 2, WSHeads::lexical, WSHeads::closedclass, WSHeads::syntactic));
    push_back(new WSHeads(0, true, 2, WSHeads::closedclass, WSHeads::lexical, WSHeads::syntactic));
    push_back(new WSHeads(0, true, 2, WSHeads::lexical, WSHeads::lexical, WSHeads::syntactic));
    push_back(new WSHeads(0, true, 2, WSHeads::lexical, WSHeads::lexical, WSHeads::semantic));
    push_back(new WSHeads(0, true, 3, WSHeads::pos, WSHeads::pos, WSHeads::syntactic));
    push_back(new WSHeads(0, true, 3, WSHeads::pos, WSHeads::pos, WSHeads::semantic));
    push_back(new WSHeads(0, true, 3, WSHeads::pos, WSHeads::closedclass, WSHeads::syntactic));
    push_back(new WSHeads(0, true, 3, WSHeads::closedclass, WSHeads::pos, WSHeads::syntactic));
    push_back(new WSHeads(0, true, 3, WSHeads::closedclass, WSHeads::closedclass, WSHeads::syntactic));
  }

  if (edgefeatures) {
    typedef std::vector<WSEdges::E> Es;
    Es es;   //!< flags specifying how far to look around each constituent boundary

    WSEdges::E empty(0,0,0,0), punct1(1,0,0,0), pos1(1,1,0,0), closed1(1,1,1,0), word1(1,1,1,1);

    es.push_back(punct1);
    es.push_back(pos1);
    es.push_back(closed1);
    es.push_back(word1);
    es.push_back(WSEdges::E(2,0,0,0));
    es.push_back(WSEdges::E(2,1,0,0));
    es.push_back(WSEdges::E(2,1,1,0));

    for (size_type binflag = 0; binflag < 2; ++binflag) {
      if ((binflag == 1 && edgefeatures == 1) || (binflag==0 && edgefeatures==2))
	continue;

      push_back(new WSEdges(empty, empty, empty, empty, binflag));
      // specialized features looking for punctuation surrounding constituent
      push_back(new WSEdges(WSEdges::E(2,0,0,0), empty, empty, WSEdges::E(2,0,0,0), binflag));

      cforeach (Es, it, es) {
	push_back(new WSEdges(*it, empty, empty, empty, binflag));
	push_back(new WSEdges(empty, *it, empty, empty, binflag));
	push_back(new WSEdges(empty, empty, *it, empty, binflag));
	push_back(new WSEdges(empty, empty, empty, *it, binflag));

	push_back(new WSEdges(*it, empty, empty, punct1, binflag));
	push_back(new WSEdges(empty, *it, empty, punct1, binflag));
	push_back(new WSEdges(empty, empty, *it, punct1, binflag));

	push_back(new WSEdges(*it, empty, empty, pos1, binflag));
	push_back(new WSEdges(empty, *it, empty, pos1, binflag));
	push_back(new WSEdges(empty, empty, *it, pos1, binflag));

	push_back(new WSEdges(*it, empty, empty, closed1, binflag));
	push_back(new WSEdges(empty, *it, empty, closed1, binflag));
	push_back(new WSEdges(empty, empty, *it, closed1, binflag));
      }
    }
  }

  if (ngram) {
    push_back(new NGram(1, 1, false, false));
    push_back(new NGram(1, 1, false, true));
    push_back(new NGram(1, 1, true, false));
    push_back(new NGram(1, 1, true, true));
    push_back(new NGram(2, 1, true, true));
    push_back(new NGram(3, 1, true, true));
    push_back(new NGram(1, 1, false, false, NGram::lexical, NGram::none));
    push_back(new NGram(1, 1, false, false, NGram::none, NGram::lexical));
    push_back(new NGram(1, 1, false, false, NGram::lexical, NGram::lexical));
    push_back(new NGram(2, 1, false, false, NGram::lexical, NGram::none));
    push_back(new NGram(2, 1, false, false, NGram::none, NGram::lexical));
    push_back(new NGram(1, 1, true, false, NGram::lexical, NGram::none));
    push_back(new NGram(1, 1, true, false, NGram::none, NGram::lexical));
    push_back(new NGram(1, 1, true, false, NGram::lexical, NGram::lexical));
    push_back(new NGram(2, 1, true, false, NGram::lexical, NGram::none));
    push_back(new NGram(2, 1, true, false, NGram::none, NGram::lexical));
    push_back(new NGram(1, 1, false, true, NGram::lexical, NGram::none));
    push_back(new NGram(1, 1, false, true, NGram::none, NGram::lexical));
    push_back(new NGram(1, 1, false, true, NGram::lexical, NGram::lexical));
    push_back(new NGram(2, 1, false, true, NGram::lexical, NGram::none));
    push_back(new NGram(2, 1, false, true, NGram::none, NGram::lexical));
  }

  if (ngramtree) {
    push_back(new NGramTree(2, NGramTree::none, true));
    push_back(new NGramTree(2, NGramTree::functional, true));
    push_back(new NGramTree(2, NGramTree::all, true));
    push_back(new NGramTree(3, NGramTree::none, true));
    push_back(new NGramTree(3, NGramTree::functional, true));
    push_back(new NGramTree(3, NGramTree::all, true));
    push_back(new NGramTree(4, NGramTree::none, true));
    push_back(new NGramTree(4, NGramTree::functional, true));
  }

  if (rbcontext) {
    push_back(new RBContext(false, false, false));
    push_back(new RBContext(false, false, true));
    push_back(new RBContext(false, true, false));
    push_back(new RBContext(false, true, true));
    push_back(new RBContext(true, false, false));
    push_back(new RBContext(true, false, true));
    push_back(new RBContext(true, true, false));
    push_back(new RBContext(true, true, true));
  }
  
}  // FeatureClassPtrs::wsfeatures()


inline void FeatureClassPtrs::nfeatures() {
  push_back(new NLogP());
  push_back(new RightBranch());
  push_back(new Heavy());

  push_back(new CoPar(false));
  push_back(new CoPar(true));
  push_back(new CoLenPar());

  push_back(new Word(1));
  push_back(new Word(2));

  push_back(new WProj());
  
  push_back(new WSHeads(0, true, 2, WSHeads::pos, WSHeads::pos, WSHeads::syntactic));
  push_back(new WSHeads(0, true, 2, WSHeads::pos, WSHeads::closedclass, WSHeads::syntactic));
  push_back(new WSHeads(0, true, 2, WSHeads::closedclass, WSHeads::pos, WSHeads::syntactic));
  push_back(new WSHeads(0, true, 2, WSHeads::closedclass, WSHeads::closedclass, WSHeads::syntactic));
  push_back(new WSHeads(0, true, 2, WSHeads::lexical, WSHeads::closedclass, WSHeads::syntactic));
  push_back(new WSHeads(0, true, 2, WSHeads::closedclass, WSHeads::lexical, WSHeads::syntactic));
  push_back(new WSHeads(0, true, 2, WSHeads::lexical, WSHeads::lexical, WSHeads::syntactic));
  push_back(new WSHeads(0, true, 2, WSHeads::lexical, WSHeads::lexical, WSHeads::semantic));
  push_back(new WSHeads(0, true, 3, WSHeads::pos, WSHeads::pos, WSHeads::syntactic));
  push_back(new WSHeads(0, true, 3, WSHeads::pos, WSHeads::pos, WSHeads::semantic));
  push_back(new WSHeads(0, true, 3, WSHeads::pos, WSHeads::closedclass, WSHeads::syntactic));
  push_back(new WSHeads(0, true, 3, WSHeads::closedclass, WSHeads::pos, WSHeads::syntactic));
  push_back(new WSHeads(0, true, 3, WSHeads::closedclass, WSHeads::closedclass, WSHeads::syntactic));

  push_back(new RBContext(false, false, false));
  push_back(new RBContext(false, true, false));
  push_back(new RBContext(false, true, true));
  push_back(new RBContext(true, false, false));
  push_back(new RBContext(true, true, false));
  push_back(new RBContext(true, true, true));

  push_back(new RBContext(false, false, false, RBContext::semantic));
  push_back(new RBContext(true, false, false, RBContext::semantic));
  push_back(new RBContext(true, true, true, RBContext::semantic));

  push_back(new Rule(0, 1));
  push_back(new Rule(1, 0));
  push_back(new Rule(1, 1));
  push_back(new Rule(0, 2));
  push_back(new Rule(0, 0, true));
  push_back(new Rule(0, 0, false, true));
  push_back(new Rule(0, 0, false, false, Rule::lexical));
  push_back(new Rule(0, 0, false, false, Rule::none, Rule::lexical));
  push_back(new Rule(0, 0, false, false, Rule::lexical, Rule::lexical));

  push_back(new NGram(1, 1, false, true));
  push_back(new NGram(2, 1, false, false));
  push_back(new NGram(2, 1, true, true));
  push_back(new NGram(3, 1, false, false));
  push_back(new NGram(3, 1, true, true));
  push_back(new NGram(4, 1, false, false));
  push_back(new NGram(2, 1, false, false, NGram::lexical));
  push_back(new NGram(2, 1, false, false, NGram::none, NGram::lexical));

  push_back(new NGramTree(2, NGramTree::none, true));
  push_back(new NGramTree(2, NGramTree::functional, true));
  push_back(new NGramTree(2, NGramTree::all, true));
  push_back(new NGramTree(3, NGramTree::none, true));
  push_back(new NGramTree(3, NGramTree::functional, true));

  push_back(new HeadTree(true, false, 0, HeadTree::syntactic));
  push_back(new HeadTree(true, false, 0, HeadTree::semantic));
  push_back(new HeadTree(true, true, 0, HeadTree::semantic));

  {
    typedef std::vector<WSEdges::E> Es;
    Es es;   //!< flags specifying how far to look around each constituent boundary

    WSEdges::E empty(0,0,0,0), punct1(1,0,0,0), pos1(1,1,0,0), closed1(1,1,1,0), word1(1,1,1,1), 
      punct2(2,0,0,0), pos2(2,1,0,0), closed2(2,1,1,0), word2(2,1,1,1);

    push_back(new WSEdges(punct1, empty, empty, empty, false));
    push_back(new WSEdges(pos1, empty, empty, empty, false));
    push_back(new WSEdges(closed1, empty, empty, empty, false));
    push_back(new WSEdges(punct1, empty, punct1, punct1, false));
    push_back(new WSEdges(punct1, empty, punct1, punct1, true));
    push_back(new WSEdges(closed1, closed1, empty, empty, false));
    push_back(new WSEdges(closed1, closed1, empty, empty, true));
    push_back(new WSEdges(closed1, closed1, punct1, punct1, false));
    push_back(new WSEdges(word1, word1, empty, empty, false));

    push_back(new WSEdges(empty, punct1, empty, empty, false));
    push_back(new WSEdges(empty, pos1, empty, empty, false));
    push_back(new WSEdges(empty, closed1, empty, empty, false));
    push_back(new WSEdges(empty, word1, empty, empty, false));
    push_back(new WSEdges(empty, punct2, empty, empty, false));
    push_back(new WSEdges(empty, pos2, empty, empty, false));
    push_back(new WSEdges(empty, closed2, empty, empty, false));
    push_back(new WSEdges(empty, punct1, empty, punct1, false));
    push_back(new WSEdges(empty, pos1, empty, punct1, false));
    push_back(new WSEdges(empty, closed1, empty, punct1, false));
    push_back(new WSEdges(empty, punct1, empty, pos1, false));
    push_back(new WSEdges(empty, pos1, empty, pos1, false));
    push_back(new WSEdges(empty, closed1, empty, pos1, false));
    push_back(new WSEdges(empty, punct1, empty, closed1, false));
    push_back(new WSEdges(empty, pos1, empty, closed1, false));
    push_back(new WSEdges(empty, closed1, empty, closed1, false));

    push_back(new WSEdges(empty, empty, punct1, empty, false));
    push_back(new WSEdges(empty, empty, pos1, empty, false));
    push_back(new WSEdges(empty, empty, closed1, empty, false));
    push_back(new WSEdges(empty, empty, word1, empty, false));
    push_back(new WSEdges(empty, empty, punct2, empty, false));
    push_back(new WSEdges(empty, empty, pos2, empty, false));
    push_back(new WSEdges(empty, empty, closed2, empty, false));
    push_back(new WSEdges(empty, empty, punct1, punct1, false));
    push_back(new WSEdges(empty, empty, pos1, punct1, false));
    push_back(new WSEdges(empty, empty, closed1, punct1, false));
    push_back(new WSEdges(empty, empty, punct1, pos1, false));
    push_back(new WSEdges(empty, empty, pos1, pos1, false));
    push_back(new WSEdges(empty, empty, closed1, pos1, false));
    push_back(new WSEdges(empty, empty, punct1, closed1, false));
    push_back(new WSEdges(empty, empty, pos1, closed1, false));
    push_back(new WSEdges(empty, empty, closed1, closed1, false));

    push_back(new WSEdges(empty, empty, empty, punct1, false));
    push_back(new WSEdges(empty, empty, empty, punct1, true));
    push_back(new WSEdges(empty, empty, empty, punct2, false));
    push_back(new WSEdges(empty, empty, empty, pos1, false));
    push_back(new WSEdges(empty, empty, empty, pos1, true));
    push_back(new WSEdges(empty, empty, empty, pos2, false));
    push_back(new WSEdges(empty, empty, empty, closed1, false));
    push_back(new WSEdges(empty, empty, empty, closed1, true));
    push_back(new WSEdges(empty, empty, empty, closed2, false));
    push_back(new WSEdges(empty, empty, empty, word1, false));
    push_back(new WSEdges(empty, empty, empty, word1, true));
    push_back(new WSEdges(empty, empty, empty, word2, false));
  }
}  // FeatureClassPtrs::nfeatures()

inline void FeatureClassPtrs::sfeatures() {
  push_back(new NLogP());
  push_back(new RightBranch());
  push_back(new Heavy());

  push_back(new CoPar(false));

  push_back(new RBContext(false, true, false));
  push_back(new RBContext(false, true, true));
  push_back(new RBContext(true, false, false));
  push_back(new RBContext(true, true, false));
  push_back(new RBContext(true, true, true));

  push_back(new Rule(0, 0, true));
  push_back(new Rule(0, 0, false, false, Rule::lexical, Rule::lexical));

  push_back(new NGram(2, 1, false, false));
  push_back(new NGram(2, 1, false, false, NGram::none, NGram::lexical));

  push_back(new WProj());

  push_back(new NGramTree(2, NGramTree::all, true));

  push_back(new HeadTree(true, false, 0, HeadTree::syntactic));
  
  push_back(new WSHeads(0, true, 2, WSHeads::lexical, WSHeads::lexical, WSHeads::semantic));
  push_back(new WSHeads(0, true, 3, WSHeads::pos, WSHeads::pos, WSHeads::semantic));
  push_back(new WSHeads(0, true, 3, WSHeads::closedclass, WSHeads::closedclass, WSHeads::syntactic));

  {
    typedef std::vector<WSEdges::E> Es;
    Es es;   //!< flags specifying how far to look around each constituent boundary

    WSEdges::E empty(0,0,0,0), punct1(1,0,0,0), pos1(1,1,0,0), closed1(1,1,1,0), word1(1,1,1,1), 
      punct2(2,0,0,0), pos2(2,1,0,0), closed2(2,1,1,0), word2(2,1,1,1);

    push_back(new WSEdges(closed1, empty, empty, empty, false));
    push_back(new WSEdges(punct1, empty, punct1, punct1, false));
    push_back(new WSEdges(punct1, empty, punct1, punct1, true));
    push_back(new WSEdges(closed1, closed1, empty, empty, false));
    push_back(new WSEdges(closed1, closed1, empty, empty, true));
    push_back(new WSEdges(word1, word1, empty, empty, false));

    push_back(new WSEdges(empty, closed1, empty, empty, false));
    push_back(new WSEdges(empty, word1, empty, empty, false));
    push_back(new WSEdges(empty, punct2, empty, empty, false));
    push_back(new WSEdges(empty, closed1, empty, punct1, false));
    push_back(new WSEdges(empty, punct1, empty, closed1, false));

    push_back(new WSEdges(empty, empty, punct1, empty, false));
    push_back(new WSEdges(empty, empty, punct2, empty, false));
    push_back(new WSEdges(empty, empty, punct1, punct1, false));
    push_back(new WSEdges(empty, empty, punct1, closed1, false));
    push_back(new WSEdges(empty, empty, pos1, closed1, false));

    push_back(new WSEdges(empty, empty, empty, punct1, false));
    push_back(new WSEdges(empty, empty, empty, punct2, false));
    push_back(new WSEdges(empty, empty, empty, pos1, false));
    push_back(new WSEdges(empty, empty, empty, pos1, true));
    push_back(new WSEdges(empty, empty, empty, closed1, false));
    push_back(new WSEdges(empty, empty, empty, closed2, false));
  }
}  // FeatureClassPtrs::sfeatures()

//! FeatureClassPtrs::FeatureClassPtrs() preloads a
//! set of features.
//
inline FeatureClassPtrs::FeatureClassPtrs(const char* fcname) {
  if (fcname == NULL || strcmp(fcname, "nfeatures") == 0)
    nfeatures();
  else if (strcmp(fcname, "sfeatures") == 0)
    sfeatures();
  else if (strcmp(fcname, "wshead") == 0)    // wsfeatures(bool headfeatures, bool edgefeatures, bool ngram, bool ngramtree, bool rbcontext) 
    wsfeatures(true, false, false, false, false);
  else if (strcmp(fcname, "wsedge") == 0)
    wsfeatures(false, 3, false, false, false);
  else if (strcmp(fcname, "wsedge0") == 0)
    wsfeatures(false, 1, false, false, false);
  else if (strcmp(fcname, "wsedge1") == 0)
    wsfeatures(false, 2, false, false, false);
  else if (strcmp(fcname, "wsngram") == 0)
    wsfeatures(false, false, true, false, false);
  else if (strcmp(fcname, "wsngramtree") == 0)
    wsfeatures(false, false, false, true, false);
  else if (strcmp(fcname, "wsrbcontext") == 0)
    wsfeatures(false, false, false, false, true);
  else if (strcmp(fcname, "conll") == 0)
    features_connll();
  else if (strcmp(fcname, "splh") == 0)
    features_splh(false);
  else if (strcmp(fcname, "splhnn") == 0)
    features_splh(false, true);
  else if (strcmp(fcname, "splhlocal") == 0)
    features_splh(true);
  else if (strcmp(fcname, "splhsuffix0") == 0)
    features_splhsuffix(0);
  else if (strcmp(fcname, "splhsuffix1") == 0)
    features_splhsuffix(1);
  else if (strcmp(fcname, "splhsuffix3") == 0)
    features_splhsuffix(3);
  else if (strcmp(fcname, "wedges") == 0)
    features_wedges();
  else if (strcmp(fcname, "ws") == 0)
    wsfeatures();
  else if (strcmp(fcname, "wsall") == 0)
    wsfeatures(true, 3, true, true, true);
  else {
    std::cerr << "## Error in nfeatures.h: FeatureClassPtrs::FeatureClassPtrs(), unknown fcname = "
	      << fcname << std::endl;
    exit(EXIT_FAILURE);
  }
  if (debug_level >= 0)
    std::cerr << "# There are " << size() << " feature classes." << std::endl;
} // FeatureClassPtrs::FeatureClassPtrs()
    

#undef FloatTol

#endif // NFEATURES_H
