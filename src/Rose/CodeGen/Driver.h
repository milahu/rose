
#ifndef Rose_CodeGen_Driver_h
#define Rose_CodeGen_Driver_h

#include <set>
#include <map>
#include <string>

#include <boost/filesystem.hpp>

class SgProject;
class SgSourceFile;
class SgScopeStatement;
class SgSymbol;
class SgGlobal;

namespace Rose { namespace CodeGen {

/**
 * \brief facilitates the manipulation of source-files (esp. library headers and generated files).
 */
class Driver {
  public:
    SgProject * project; //!< the Rose project

  private:
    size_t file_id_counter;
    std::map<boost::filesystem::path, size_t> path_to_id_map;
    std::map<size_t, SgSourceFile *> id_to_file_map;
    std::map<SgSourceFile *, size_t> file_to_id_map;
    std::map<size_t, std::set<size_t> > file_id_to_accessible_file_id_map;

    void init(SgProject * project_);
    size_t add(SgSourceFile * file);

  public:
    Driver();
    Driver(SgProject * project_);
    Driver(std::vector<std::string> & args);

    //! Create a file ()
    size_t create(const boost::filesystem::path & path);
    size_t add(const boost::filesystem::path & path);

    //! Retrieve file-id of associated with a file
    //! @{
    size_t getFileID(const boost::filesystem::path & path) const;
    size_t getFileID(SgSourceFile * source_file) const;
    size_t getFileID(SgScopeStatement * scope) const;
    //! @}

    //! Retrieve the global scope of a file from its file-id
    SgGlobal * getGlobalScope(size_t file_id) const;

    //! Set a file to be unparsed with the project (by default file added to the driver are *NOT* unparsed)
    void setUnparsedFile(size_t file_id) const;

    //! Set a file to be compiled with the project (by default file added to the driver are *NOT* compiled)
    void setCompiledFile(size_t file_id) const;

    //! Add include statement for header inside of target.
    void addIncludeDirectives(size_t target_file_id, size_t header_file_id);

    /// Import external header for a given file
    void addExternalHeader(size_t file_id, std::string header_name, bool is_system_header = true);

    /// Add a pragma at the begining of the file
    void addPragmaDecl(size_t file_id, std::string str);
};

} }

#endif /* Rose_CodeGen_Driver_h */

