/* JVM File Header (SgAsmJvmFileHeader and related classes) */
#include <featureTests.h>
#ifdef ROSE_ENABLE_BINARY_ANALYSIS
#include "sage3basic.h"

#include "Jvm.h"

using std::cout;
using std::endl;

SgAsmJvmFileHeader::SgAsmJvmFileHeader(SgAsmGenericFile* f)
  : SgAsmGenericHeader{f}, p_minor_version{0}, p_major_version{0}, p_access_flags{0},
    p_this_class{0}, p_super_class{0}, p_constant_pool{nullptr}, p_field_table{nullptr},
    p_method_table{nullptr}, p_attribute_table{nullptr}
{
  ASSERT_not_null(f);
  set_parent(f);

  // Check that the file has already been parsed and has content
  ASSERT_not_null(get_file());
  ROSE_ASSERT(get_size() > 0);

  set_name(new SgAsmBasicString("JVM File Header"));
  set_synthesized(true);
  set_purpose(SP_HEADER); // SectionPurpose

  /* Magic number */
  p_magic.clear();
  p_magic.push_back(0xCA);
  p_magic.push_back(0xFE);
  p_magic.push_back(0xBA);
  p_magic.push_back(0xBE);

  /* Executable Format */
  ASSERT_not_null(p_exec_format);
  p_exec_format->set_family(FAMILY_JVM);
  p_exec_format->set_purpose(PURPOSE_EXECUTABLE);
  p_exec_format->set_sex(ByteOrder::ORDER_MSB);
  p_exec_format->set_word_size(4);
  p_exec_format->set_version(0);
  p_exec_format->set_is_current_version(false);
  p_exec_format->set_abi(ABI_JVM);
  p_exec_format->set_abi_version(0);

  p_isa = ISA_JVM;
}

SgAsmJvmFileHeader*
SgAsmJvmFileHeader::parse()
{
  SgAsmGenericFile* gf = get_file();
  ASSERT_not_null(gf);
  ROSE_ASSERT(get_size() > 0);

  auto header = gf->get_header(SgAsmGenericFile::FAMILY_JVM);
  ASSERT_not_null(header);
  ROSE_ASSERT(header == this);

  /* parse base class */
  SgAsmGenericHeader::parse();

  cout << "WARNING: SgAsmJvmFileHeader::parse() ...\n"; // TODO: move to constructor (probably can't because parse needed
  // how does this offset fit in now that switched to file header from class file?
  rose_addr_t offset{get_offset()};

  /* Construct, but don't parse (yet), constant pool */
  auto pool = new SgAsmJvmConstantPool(this);
  set_constant_pool(pool);

  /* Ensure magic number in file is correct */
  unsigned char magic[4];
  auto count = read_content(offset, magic, sizeof magic);
  if (4!=count || p_magic.size()!=count || p_magic[0]!=magic[0]
               || p_magic[1]!=magic[1] || p_magic[2]!=magic[2] || p_magic[3]!=magic[3]) {
    throw FormatError("Bad Java class file magic number");
  }
  offset += count;
  set_offset(offset);

  /* Minor and major version */
  Jvm::read_value(pool, p_minor_version);
  Jvm::read_value(pool, p_major_version);

  ASSERT_not_null(p_exec_format);
  p_exec_format->set_version(p_major_version);
  p_exec_format->set_is_current_version(true);
  p_exec_format->set_abi_version(p_major_version);

  /* And finally the constant pool can be parsed */
  pool->parse();

  Jvm::read_value(pool, p_access_flags);
  Jvm::read_value(pool, p_this_class);
  Jvm::read_value(pool, p_super_class);

  uint16_t interfaces_count;
  Jvm::read_value(pool, interfaces_count);

  std::list<uint16_t>& interfaces = get_interfaces();
  for (int i = 0; i < interfaces_count; i++) {
    uint16_t index;
    Jvm::read_value(pool, index);
    interfaces.push_back(index);
#ifdef DEBUG_ON
    cout << "SgAsmJvmFileHeader::parse(): &p_interfaces: " << &p_interfaces << ": size:" << p_interfaces.size() << endl;;
    cout << "SgAsmJvmFileHeader::parse(): get_interfaces().size(): " << get_interfaces().size() << endl;;
    cout << "SgAsmJvmFileHeader::parse(): interfaces.size(): " << interfaces.size() << endl;;
#endif
  }

  /* Fields */
  auto fields = new SgAsmJvmFieldTable(this);
  set_field_table(fields);
  fields->parse();

  /* Methods */
  auto methods = new SgAsmJvmMethodTable(this);
  set_method_table(methods);
  methods->parse();

  /* Attributes */
  auto attributes = new SgAsmJvmAttributeTable(this, /*parent*/this);
  set_attribute_table(attributes);
  ASSERT_not_null(attributes->get_parent());
  attributes->parse(pool);

#ifdef DEBUG_ON
  cout << "SgAsmJvmFileHeader::parse() offset, get_offset: " << offset << ", " << get_offset() << endl;
#endif

  if (1 != (get_end_offset() - get_offset())) {
    ROSE_ASSERT(false && "Error reading file, end of file not reached");
  }

  return this;
}

#if 0
bool
SgAsmJvmFileHeader::reallocate()
{
  /* Do not reallocate this file header. */
  return false;
}
#endif

#if 0
void
SgAsmJvmFileHeader::unparse(std::ostream &f) const
{
  /* Do not unparse to this file. */
}

bool
SgAsmJvmFileHeader::is_JVM(SgAsmGenericFile* file)
{
  ASSERT_not_null(file);

  /* Turn off byte reference tracking for the duration of this function. We don't want our testing the file contents to
   * affect the list of bytes that we've already referenced or which we might reference later. */
  bool was_tracking = file->get_tracking_references();
  file->set_tracking_references(false);
  try {
    unsigned char magic[4];
    auto count = file->read_content(0, magic, sizeof magic);
    if (4 != count || 0xCA!=magic[0] || 0xFE!=magic[1] || 0xBA!=magic[2] || 0xBE!=magic[3]) {
      throw 1;
    }
  } catch (...) {
    file->set_tracking_references(was_tracking);
    return false;
  }
  file->set_tracking_references(was_tracking);
  return true;
}

const char *
SgAsmJvmFileHeader::format_name() const {
  return "JVM";
}
#endif // add to ROSETTA

#endif
