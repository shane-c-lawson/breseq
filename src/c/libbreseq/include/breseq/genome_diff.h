#ifndef _BRESEQ_GENOME_DIFF_H_
#define _BRESEQ_GENOME_DIFF_H_

#include <map>
#include <string>
#include <utility>
#include <vector>
#include <sstream>
#include <iomanip>
#include <cmath>
#include <boost/variant.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/shared_ptr.hpp>

namespace breseq {

	// Common keywords used for diff entries:
	extern const char* TYPE;
	extern const char* ID;
	extern const char* PID;
	extern const char* SEQ_ID;
	extern const char* START;
	extern const char* END;
	extern const char* START_RANGE;
	extern const char* END_RANGE;
	extern const char* LEFT_OUTSIDE_COV;
	extern const char* LEFT_INSIDE_COV;
	extern const char* RIGHT_INSIDE_COV;
	extern const char* RIGHT_OUTSIDE_COV;
	extern const char* POSITION;
	extern const char* INSERT_POSITION;
	extern const char* QUALITY;
	extern const char* REF_BASE;
	extern const char* NEW_BASE;
	extern const char* FREQUENCY;
	extern const char* REJECT;
	extern const char* REF_COV;
	extern const char* NEW_COV;
	extern const char* TOT_COV;
	
	// Types of diff entries:
	extern const char* RA;
	extern const char* MC;
	extern const char* UN;
	
	//! Convenience typedef, used during diff entry marshalling.
	typedef std::vector<std::string> field_list_t;

	
	/*! Genome diff entry type.
	 
	 Instead of trying to define (and maintain!) concrete classes for each different 
	 type of diff entry, we instead define a single generic diff entry, and then 
	 provide easy-to-use mechanisms to manipulate them.
	 
	 The key abstraction here is that a diff entry is a map of keys (strings) to
	 values.  In this case, values are boost::variants (discriminated unions) that
	 provide int, double, and string types.
	 
	 For convenience, there are also factory methods on genome_diff that create diff
	 entries that have been pre-populated according to their type.
	 */
	struct diff_entry {
		typedef std::string key_t; //!< Diff entry keys.
		typedef boost::variant<char,uint8_t,uint32_t,int,double,std::string,std::pair<int,int> > value_t; //!< Diff entry values.
		typedef std::map<key_t, value_t> map_t; //!< Diff entry key-value map.
		
		//! Constructor.
		diff_entry(const std::string& t, const std::string& id, const std::string& parents);
		
		//! Destructor.
		virtual ~diff_entry() { }
		
		//! Ease-of-use accessor for setting fields on this entry.
		value_t& operator[](const key_t& k) { return _fields[k]; }
				
		//! Marshal this diff entry into an ordered list of fields.
		virtual void marshal(field_list_t& s);
		
		//! Clone this entry.
		virtual diff_entry* clone() const = 0;
		
		map_t _fields; //!< Information about this diff entry.
		std::string _type;
		std::string _id;
		std::string _parents;
	};
	
	
	//! Output operator for a diff entry.
	std::ostream& operator<<(std::ostream& out, diff_entry& de);
	
	
	/*!
	 */
	struct ra : public diff_entry {
		//! Constructor.
		ra(const std::string& id, const std::string& parents);

		//! Clone this entry.
		virtual diff_entry* clone() const { return new ra(*this); }
	};

	
	/*!
	 */
	struct mc : public diff_entry {
		//! Constructor.
		mc(const std::string& id, const std::string& parents);
	
		//! Clone this entry.
		virtual diff_entry* clone() const { return new mc(*this); }
	};
	
	
	/*!
	 */
	struct un : public diff_entry {
		//! Constructor.
		un(const std::string& id, const std::string& parents);

		//! Clone this entry.
		virtual diff_entry* clone() const { return new un(*this); }
	};
	
	
	/*! Visitor class to gather diff entries.
	 */
	struct string_visitor : public boost::static_visitor< > {

		//! Constructor.
		string_visitor() { }
		
		//! Default output.
		template <typename T>
		void operator()(T& op) {
			_s = boost::lexical_cast<std::string>(op);
		}
		
		//! Double-output.
		void operator()(double& v) {
			using namespace std;
			if(std::isnan(v)) {
				_s = "NA";
			} else {
				ostringstream interpreter;
				interpreter << fixed << setprecision(1) << v;
				_s = interpreter.str();
			}
		}
		
		//! Pairs are handled separately.
    void operator()(std::pair<int,int>& p) {
			_s = boost::lexical_cast<std::string>(p.first) + "/" + boost::lexical_cast<std::string>(p.second);
		}
		
		//! Retrieve the string that was built during visitation.
		const std::string& str() { return _s; }
		
		std::string _s;
	};
	
	
	/*! Genome diff class.
	 
	 //			Genome Diff files are tab delimitted. The first column defines the type of entry on a given line.
	 //			The second and third columns are type-nonspecific (id, parents), followed by type-specific
	 //			columns, then an arbitrary number of columns of more detailed data in a key=value format.
	 
	 */
	class genome_diff {
	public:

		typedef std::vector<boost::shared_ptr<diff_entry> > entry_list_t; //!< Type for a list of diff entries.
		
		//! Constructor.
		genome_diff() : _current_id(0) { }
		
		//! Constructor that sets a default filename.
		genome_diff(const std::string& filename) : _default_filename(filename), _current_id(0) { }
		
		//! Destructor.
		~genome_diff() { }

		//! Retrieve a new diff entry id for this genome diff.
		unsigned int new_id() { return ++_current_id; }
		
		//! Add evidence to this genome diff.
		void add(const diff_entry& v);

		//! Read a genome diff from the default file.
		void read() { read(_default_filename); }

		//! Write the genome diff to the default file.
		void write() { write(_default_filename); }

		//! Read a genome diff from a file.
		void read(const std::string& filename);
		
		//! Write the genome diff to a file.
		void write(const std::string& filename);

	protected:		
		const std::string _default_filename; //!< Default filename for this diff.
		entry_list_t _entry_list; //!< All diff entries.
		unsigned int _current_id; //!< Smallest available id.
	};
	
}

#endif