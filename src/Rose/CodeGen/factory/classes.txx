
#ifndef Rose_CodeGen_factory_classes_txx
#define Rose_CodeGen_factory_classes_txx

namespace Rose { namespace CodeGen {

template <typename CRT, typename API>
struct __factory_helper_t<CRT, API, Object::a_class> {
  template <typename... Args>
  static declaration_t<Object::a_class> * instantiate(
      Factory<CRT, API> & factory,
      symbol_t<Object::a_class> * sym,
      SgClassType * parent,
      Args... args
  );

  template <typename... Args>
  static reference_t<Object::a_class> * reference(
      Factory<CRT, API> & factory,
      symbol_t<Object::a_class> * sym,
      SgClassType * parent,
      Args... args
  );
};

template <typename CRT, typename API>
template <typename... Args>
declaration_t<Object::a_class> * __factory_helper_t<CRT, API, Object::a_class>::instantiate(
  Factory<CRT, API> & factory,
  symbol_t<Object::a_class> * sym,
  SgClassType * parent,
  Args... args
) {
  ROSE_ABORT(); // TODO
  return nullptr;
}

template <typename CRT, typename API>
template <typename... Args>
reference_t<Object::a_class> * __factory_helper_t<CRT, API, Object::a_class>::reference(
  Factory<CRT, API> & factory,
  symbol_t<Object::a_class> * sym,
  SgClassType * parent,
  Args... args
) {
  ROSE_ABORT(); // TODO
  return nullptr;
}

} }

#endif /* Rose_CodeGen_factory_classes_txx */

