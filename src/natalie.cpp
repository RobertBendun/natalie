#include "natalie.hpp"
#include "natalie/forward.hpp"
#include "natalie/value.hpp"
#include <ctype.h>
#include <math.h>
#include <stdarg.h>
#include <sys/wait.h>

namespace Natalie {

Env *build_top_env() {
    auto *global_env = GlobalEnv::the();
    auto *env = new Env {};
    global_env->set_main_env(env);

    ClassObject *Class = ClassObject::bootstrap_class_class(env);
    global_env->set_Class(Class);

    ClassObject *BasicObject = ClassObject::bootstrap_basic_object(env, Class);

    ClassObject *Object = BasicObject->subclass(env, "Object");

    global_env->set_Object(Object);

    ClassObject *Symbol = Object->subclass(env, "Symbol", Object::Type::Symbol);
    global_env->set_Symbol(Symbol);
    Object->const_set(SymbolObject::intern("Symbol"), Symbol);

    // these must be defined after Object exists
    Object->const_set(SymbolObject::intern("Class"), Class);
    Object->const_set(SymbolObject::intern("BasicObject"), BasicObject);
    Object->const_set(SymbolObject::intern("Object"), Object);

    ClassObject *Module = Object->subclass(env, "Module", Object::Type::Module);
    global_env->set_Module(Module);
    Object->const_set(SymbolObject::intern("Module"), Module);
    Class->set_superclass_DANGEROUSLY(Module);
    Class->set_singleton_class(Module->singleton_class()->subclass(env, "#<Class:Class>"));

    ModuleObject *Kernel = new ModuleObject { "Kernel" };
    Object->const_set(SymbolObject::intern("Kernel"), Kernel);
    Object->include_once(env, Kernel);

    ModuleObject *Comparable = new ModuleObject { "Comparable" };
    Object->const_set(SymbolObject::intern("Comparable"), Comparable);

    ModuleObject *Enumerable = new ModuleObject { "Enumerable" };
    Object->const_set(SymbolObject::intern("Enumerable"), Enumerable);

    BasicObject->define_singleton_method(env, SymbolObject::intern("new"), Object::_new, -1);

    ClassObject *NilClass = Object->subclass(env, "NilClass", Object::Type::Nil);
    Object->const_set(SymbolObject::intern("NilClass"), NilClass);
    NilObject::the();

    ClassObject *TrueClass = Object->subclass(env, "TrueClass", Object::Type::True);
    Object->const_set(SymbolObject::intern("TrueClass"), TrueClass);
    TrueObject::the();

    ClassObject *FalseClass = Object->subclass(env, "FalseClass", Object::Type::False);
    Object->const_set(SymbolObject::intern("FalseClass"), FalseClass);
    FalseObject::the();

    ClassObject *Fiber = Object->subclass(env, "Fiber", Object::Type::Fiber);
    Object->const_set(SymbolObject::intern("Fiber"), Fiber);

    ClassObject *Numeric = Object->subclass(env, "Numeric");
    Object->const_set(SymbolObject::intern("Numeric"), Numeric);
    Numeric->include_once(env, Comparable);

    ClassObject *Integer = Numeric->subclass(env, "Integer", Object::Type::Integer);
    global_env->set_Integer(Integer);
    Object->const_set(SymbolObject::intern("Integer"), Integer);
    Object->const_set(SymbolObject::intern("Fixnum"), Integer);

    ClassObject *Float = Numeric->subclass(env, "Float", Object::Type::Float);
    global_env->set_Float(Float);
    Object->const_set(SymbolObject::intern("Float"), Float);
    Float->include_once(env, Comparable);
    FloatObject::build_constants(env, Float);

    Value Math = new ModuleObject { "Math" };
    Object->const_set(SymbolObject::intern("Math"), Math);
    Math->const_set(SymbolObject::intern("PI"), new FloatObject { M_PI });

    ClassObject *String = Object->subclass(env, "String", Object::Type::String);
    global_env->set_String(String);
    Object->const_set(SymbolObject::intern("String"), String);
    String->include_once(env, Comparable);

    ClassObject *Array = Object->subclass(env, "Array", Object::Type::Array);
    global_env->set_Array(Array);
    Object->const_set(SymbolObject::intern("Array"), Array);
    Array->include_once(env, Enumerable);

    ClassObject *Binding = Object->subclass(env, "Binding", Object::Type::Binding);
    global_env->set_Binding(Binding);
    Object->const_set(SymbolObject::intern("Binding"), Binding);

    ClassObject *Hash = Object->subclass(env, "Hash", Object::Type::Hash);
    global_env->set_Hash(Hash);
    Object->const_set(SymbolObject::intern("Hash"), Hash);
    Hash->include_once(env, Enumerable);

    ClassObject *Random = Object->subclass(env, "Random", Object::Type::Random);
    global_env->set_Random(Random);
    Object->const_set(SymbolObject::intern("Random"), Random);
    Random->const_set(SymbolObject::intern("DEFAULT"), (new RandomObject)->initialize(env, nullptr));

    ClassObject *Regexp = Object->subclass(env, "Regexp", Object::Type::Regexp);
    global_env->set_Regexp(Regexp);
    Object->const_set(SymbolObject::intern("Regexp"), Regexp);
    Regexp->const_set(SymbolObject::intern("IGNORECASE"), Value::integer(1));
    Regexp->const_set(SymbolObject::intern("EXTENDED"), Value::integer(2));
    Regexp->const_set(SymbolObject::intern("MULTILINE"), Value::integer(4));

    ClassObject *Range = Object->subclass(env, "Range", Object::Type::Range);
    Object->const_set(SymbolObject::intern("Range"), Range);
    Range->include_once(env, Enumerable);

    ClassObject *MatchData = Object->subclass(env, "MatchData", Object::Type::MatchData);
    Object->const_set(SymbolObject::intern("MatchData"), MatchData);

    ClassObject *Proc = Object->subclass(env, "Proc", Object::Type::Proc);
    Object->const_set(SymbolObject::intern("Proc"), Proc);

    ClassObject *IO = Object->subclass(env, "IO", Object::Type::Io);
    Object->const_set(SymbolObject::intern("IO"), IO);

    ClassObject *File = IO->subclass(env, "File");
    Object->const_set(SymbolObject::intern("File"), File);
    FileObject::build_constants(env, File);

    ClassObject *Exception = Object->subclass(env, "Exception", Object::Type::Exception);
    Object->const_set(SymbolObject::intern("Exception"), Exception);
    ClassObject *ScriptError = Exception->subclass(env, "ScriptError", Object::Type::Exception);
    Object->const_set(SymbolObject::intern("ScriptError"), ScriptError);
    ClassObject *SyntaxError = ScriptError->subclass(env, "SyntaxError", Object::Type::Exception);
    Object->const_set(SymbolObject::intern("SyntaxError"), SyntaxError);
    ClassObject *StandardError = Exception->subclass(env, "StandardError", Object::Type::Exception);
    Object->const_set(SymbolObject::intern("StandardError"), StandardError);
    ClassObject *NameError = StandardError->subclass(env, "NameError", Object::Type::Exception);
    Object->const_set(SymbolObject::intern("NameError"), NameError);

    ClassObject *Encoding = GlobalEnv::the()->Object()->subclass(env, "Encoding");
    Object->const_set(SymbolObject::intern("Encoding"), Encoding);

    EncodingObject *EncodingAscii8Bit = new EncodingObject { Encoding::ASCII_8BIT, { "ASCII-8BIT", "BINARY" } };
    Encoding->const_set(SymbolObject::intern("ASCII_8BIT"), EncodingAscii8Bit);
    Encoding->const_set(SymbolObject::intern("BINARY"), EncodingAscii8Bit);

    Value EncodingUTF8 = new EncodingObject { Encoding::UTF_8, { "UTF-8" } };
    Encoding->const_set(SymbolObject::intern("UTF_8"), EncodingUTF8);

    Value Process = new ModuleObject { "Process" };
    Object->const_set(SymbolObject::intern("Process"), Process);

    ClassObject *Method = Object->subclass(env, "Method", Object::Type::Method);
    Object->const_set(SymbolObject::intern("Method"), Method);

    env->global_set(SymbolObject::intern("$NAT_at_exit_handlers"), new ArrayObject {});

    auto main_obj = new Natalie::Object {};
    main_obj->add_main_object_flag();
    GlobalEnv::the()->set_main_obj(main_obj);

    Value _stdin = new IoObject { STDIN_FILENO };
    env->global_set(SymbolObject::intern("$stdin"), _stdin);
    Object->const_set(SymbolObject::intern("STDIN"), _stdin);

    Value _stdout = new IoObject { STDOUT_FILENO };
    env->global_set(SymbolObject::intern("$stdout"), _stdout);
    Object->const_set(SymbolObject::intern("STDOUT"), _stdout);

    Value _stderr = new IoObject { STDERR_FILENO };
    env->global_set(SymbolObject::intern("$stderr"), _stderr);
    Object->const_set(SymbolObject::intern("STDERR"), _stderr);

    Value ENV = new Natalie::Object {};
    Object->const_set(SymbolObject::intern("ENV"), ENV);

    ClassObject *Parser = GlobalEnv::the()->Object()->subclass(env, "Parser");
    Object->const_set(SymbolObject::intern("Parser"), Parser);

    ClassObject *Sexp = Array->subclass(env, "Sexp", Object::Type::Array);
    Object->const_set(SymbolObject::intern("Sexp"), Sexp);

    Value RUBY_VERSION = new StringObject { "3.0.0" };
    Object->const_set(SymbolObject::intern("RUBY_VERSION"), RUBY_VERSION);

    Value RUBY_ENGINE = new StringObject { "natalie" };
    Object->const_set(SymbolObject::intern("RUBY_ENGINE"), RUBY_ENGINE);

    StringObject *RUBY_PLATFORM = new StringObject { ruby_platform };
    Object->const_set(SymbolObject::intern("RUBY_PLATFORM"), RUBY_PLATFORM);

    ModuleObject *GC = new ModuleObject { "GC" };
    Object->const_set(SymbolObject::intern("GC"), GC);

    init_bindings(env);

    return env;
}

Value splat(Env *env, Value obj) {
    if (obj->is_array()) {
        return new ArrayObject { *obj->as_array() };
    } else {
        return to_ary(env, obj, false);
    }
}

void run_at_exit_handlers(Env *env) {
    ArrayObject *at_exit_handlers = env->global_get(SymbolObject::intern("$NAT_at_exit_handlers"))->as_array();
    assert(at_exit_handlers);
    for (int i = at_exit_handlers->size() - 1; i >= 0; i--) {
        Value proc = (*at_exit_handlers)[i];
        assert(proc);
        assert(proc->is_proc());
        NAT_RUN_BLOCK_WITHOUT_BREAK(env, proc->as_proc()->block(), 0, nullptr, nullptr);
    }
}

void print_exception_with_backtrace(Env *env, ExceptionObject *exception) {
    IoObject *_stderr = env->global_get(SymbolObject::intern("$stderr"))->as_io();
    int fd = _stderr->fileno();
    const ArrayObject *backtrace = exception->backtrace();
    if (backtrace && backtrace->size() > 0) {
        dprintf(fd, "Traceback (most recent call last):\n");
        for (int i = backtrace->size() - 1; i > 0; i--) {
            StringObject *line = (*backtrace)[i]->as_string();
            assert(line->type() == Object::Type::String);
            dprintf(fd, "        %d: from %s\n", i, line->c_str());
        }
        StringObject *line = (*backtrace)[0]->as_string();
        dprintf(fd, "%s: ", line->c_str());
    }
    dprintf(fd, "%s (%s)\n", exception->message()->c_str(), exception->klass()->class_name_or_blank()->c_str());
}

void handle_top_level_exception(Env *env, ExceptionObject *exception, bool run_exit_handlers) {
    if (exception->is_a(env, GlobalEnv::the()->Object()->const_find(env, SymbolObject::intern("SystemExit"))->as_class())) {
        Value status_obj = exception->ivar_get(env, SymbolObject::intern("@status"));
        if (run_exit_handlers) run_at_exit_handlers(env);
        if (status_obj->type() == Object::Type::Integer) {
            nat_int_t val = status_obj->as_integer()->to_nat_int_t();
            if (val >= 0 && val <= 255) {
                clean_up_and_exit(val);
            } else {
                clean_up_and_exit(1);
            }
        } else {
            clean_up_and_exit(1);
        }
    } else {
        print_exception_with_backtrace(env, exception);
    }
}

ArrayObject *to_ary(Env *env, Value obj, bool raise_for_non_array) {
    auto to_ary_symbol = SymbolObject::intern("to_ary");
    if (obj->is_array()) {
        return obj->as_array();
    } else if (obj->respond_to(env, to_ary_symbol)) {
        Value ary = obj.send(env, to_ary_symbol);
        if (ary->is_array()) {
            return ary->as_array();
        } else if (ary->is_nil() || !raise_for_non_array) {
            ary = new ArrayObject {};
            ary->as_array()->push(obj);
            return ary->as_array();
        } else {
            auto *class_name = obj->klass()->class_name_or_blank();
            env->raise("TypeError", "can't convert {} to Array ({}#to_ary gives {})", class_name, class_name, ary->klass()->class_name_or_blank());
        }
    } else {
        ArrayObject *ary = new ArrayObject {};
        ary->push(obj);
        return ary;
    }
}

static Value splat_value(Env *env, Value value, size_t index, size_t offset_from_end, bool has_kwargs) {
    ArrayObject *splat = new ArrayObject {};
    if (has_kwargs && value->is_array() && value->as_array()->last()->is_hash())
        offset_from_end += 1; // compensate for kwargs hash that was passed as the last argument
    if (value->is_array() && index < value->as_array()->size() - offset_from_end) {
        for (size_t s = index; s < value->as_array()->size() - offset_from_end; s++) {
            splat->push((*value->as_array())[s]);
        }
    }
    return splat;
}

Value arg_value_by_path(Env *env, ArgValueByPathOptions options, size_t path_size, ...) {
    va_list args;
    va_start(args, path_size);
    bool has_default = !!options.default_value;
    auto default_value = options.default_value ?: NilObject::the();
    bool defaults_on_left = !options.defaults_on_right;
    int required_count = options.total_count - options.default_count;
    Value return_value = options.value;
    for (size_t i = 0; i < path_size; i++) {
        int index = va_arg(args, int);

        if (options.splat && i == path_size - 1) {
            va_end(args);
            return splat_value(env, return_value, index, options.offset_from_end, options.has_kwargs);
        } else {
            if (return_value->is_array()) {
                assert(return_value->as_array()->size() <= NAT_INT_MAX);
                nat_int_t ary_len = return_value->as_array()->size();

                int first_required = options.default_count;
                int remain = ary_len - required_count;

                if (has_default && index >= remain && index < first_required && defaults_on_left) {
                    // this is an arg with a default value;
                    // not enough values to fill all the required args and this one
                    va_end(args);
                    return default_value;
                }

                if (i == 0 && path_size == 1) {
                    // shift index left if needed
                    int extra_count = ary_len - required_count;
                    if (defaults_on_left && extra_count > 0 && options.default_count >= extra_count && index >= extra_count) {
                        index -= (options.default_count - extra_count);
                    } else if (ary_len <= required_count && defaults_on_left) {
                        index -= (options.default_count);
                    }
                }

                if (index < 0) {
                    // negative offset index should go from the right
                    if (ary_len >= options.total_count) {
                        index = ary_len + index;
                    } else {
                        // not enough values to fill from the right
                        // also, assume there is a splat prior to this index
                        index = options.total_count + index;
                    }
                }

                if (index < 0) {
                    // not enough values in the array, so use default
                    return_value = default_value;

                } else if (index < ary_len) {
                    // value available, yay!
                    return_value = (*return_value->as_array())[index];

                    if (has_default && options.has_kwargs && i == path_size - 1) {
                        if (return_value->is_hash()) {
                            return_value = default_value;
                        }
                    }

                } else {
                    // index past the end of the array, so use default
                    return_value = default_value;
                }

            } else if (index == 0) {
                // not an array, so nothing to do (the object itself is returned)
                // no-op

            } else {
                // not an array, and index isn't zero
                return_value = default_value;
            }
        }
    }
    va_end(args);
    return return_value;
}

Value array_value_by_path(Env *env, ArrayValueByPathOptions options, size_t path_size, ...) {
    va_list args;
    va_start(args, path_size);
    Value return_value = options.value;
    for (size_t i = 0; i < path_size; i++) {
        int index = va_arg(args, int);
        if (options.splat && i == path_size - 1) {
            va_end(args);
            return splat_value(env, return_value, index, options.offset_from_end, false);
        } else {
            if (return_value->is_array()) {

                assert(return_value->as_array()->size() <= NAT_INT_MAX);
                nat_int_t ary_len = return_value->as_array()->size();

                if (index < 0) {
                    // negative offset index should go from the right
                    index = ary_len + index;
                }

                if (index < 0) {
                    // not enough values in the array, so use default
                    return_value = options.default_value;

                } else if (index < ary_len) {
                    // value available, yay!
                    return_value = (*return_value->as_array())[index];

                } else {
                    // index past the end of the array, so use default
                    return_value = options.default_value;
                }

            } else if (index == 0) {
                // not an array, so nothing to do (the object itself is returned)
                // no-op

            } else {
                // not an array, and index isn't zero
                return_value = options.default_value;
            }
        }
    }
    va_end(args);
    return return_value;
}

Value kwarg_value_by_name(Env *env, Value args, const char *name, Value default_value) {
    return kwarg_value_by_name(env, args->as_array(), name, default_value);
}

HashObject *kwarg_hash(Value args) {
    return kwarg_hash(args->as_array());
}

HashObject *kwarg_hash(ArrayObject *args) {
    HashObject *hash;
    if (args->size() == 0) {
        hash = new HashObject {};
    } else {
        auto maybe_hash = (*args)[args->size() - 1];
        if (maybe_hash->is_hash())
            hash = maybe_hash->as_hash();
        else
            hash = new HashObject {};
    }
    return hash;
}

Value kwarg_value_by_name(Env *env, ArrayObject *args, const char *name, Value default_value) {
    auto hash = kwarg_hash(args);
    Value value = hash->as_hash()->remove(env, SymbolObject::intern(name));
    if (!value) {
        if (default_value) {
            return default_value;
        } else {
            env->raise("ArgumentError", "missing keyword: :{}", name);
        }
    }
    return value;
}

ArrayObject *args_to_array(Env *env, size_t argc, Value *args) {
    ArrayObject *ary = new ArrayObject { argc };
    for (size_t i = 0; i < argc; i++) {
        ary->push(args[i]);
    }
    return ary;
}

// much like args_to_array above, but when a block is given a single arg,
// and the block wants multiple args, call to_ary on the first arg and return that
ArrayObject *block_args_to_array(Env *env, size_t signature_size, size_t argc, Value *args) {
    if (argc == 1 && signature_size > 1) {
        return to_ary(env, args[0], true);
    }
    return args_to_array(env, argc, args);
}

void arg_spread(Env *env, size_t argc, Value *args, const char *arrangement, ...) {
    va_list va_args;
    va_start(va_args, arrangement);
    size_t len = strlen(arrangement);
    size_t arg_index = 0;
    bool optional = false;
    for (size_t i = 0; i < len; i++) {
        if (arg_index >= argc && optional)
            break;
        char c = arrangement[i];
        switch (c) {
        case '|':
            optional = true;
            break;
        case 'o': {
            Object **obj_ptr = va_arg(va_args, Object **);
            if (arg_index >= argc) env->raise("ArgumentError", "wrong number of arguments (given {}, expected {})", argc, arg_index + 1);
            Object *obj = args[arg_index++].object();
            *obj_ptr = obj;
            break;
        }
        case 'i': {
            int *int_ptr = va_arg(va_args, int *);
            if (arg_index >= argc) env->raise("ArgumentError", "wrong number of arguments (given {}, expected {})", argc, arg_index + 1);
            Value obj = args[arg_index++];
            obj->assert_type(env, Object::Type::Integer, "Integer");
            *int_ptr = obj->as_integer()->to_nat_int_t();
            break;
        }
        case 's': {
            const char **str_ptr = va_arg(va_args, const char **);
            if (arg_index >= argc) env->raise("ArgumentError", "wrong number of arguments (given {}, expected {})", argc, arg_index + 1);
            Value obj = args[arg_index++];
            if (obj == NilObject::the()) {
                *str_ptr = nullptr;
            } else {
                obj->assert_type(env, Object::Type::String, "String");
            }
            *str_ptr = obj->as_string()->c_str();
            break;
        }
        case 'b': {
            bool *bool_ptr = va_arg(va_args, bool *);
            if (arg_index >= argc) env->raise("ArgumentError", "wrong number of arguments (given {}, expected {})", argc, arg_index + 1);
            Value obj = args[arg_index++];
            *bool_ptr = obj->is_truthy();
            break;
        }
        case 'v': {
            void **void_ptr = va_arg(va_args, void **);
            if (arg_index >= argc) env->raise("ArgumentError", "wrong number of arguments (given {}, expected {})", argc, arg_index + 1);
            Value obj = args[arg_index++];
            obj = obj->ivar_get(env, SymbolObject::intern("@_ptr"));
            assert(obj->type() == Object::Type::VoidP);
            *void_ptr = obj->as_void_p()->void_ptr();
            break;
        }
        default:
            fprintf(stderr, "Unknown arg spread arrangement specifier: %%%c", c);
            abort();
        }
    }
    va_end(va_args);
}

std::pair<Value, Value> coerce(Env *env, Value lhs, Value rhs) {
    auto coerce_symbol = SymbolObject::intern("coerce");
    if (lhs->respond_to(env, coerce_symbol)) {
        Value coerced = lhs.send(env, coerce_symbol, { rhs });
        if (!coerced->is_array()) {
            env->raise("TypeError", "coerce must return [x, y]");
        }
        lhs = (*coerced->as_array())[0];
        rhs = (*coerced->as_array())[1];
        return { lhs, rhs };
    } else {
        return { rhs, lhs };
    }
}

char *zero_string(int size) {
    char *buf = new char[size + 1];
    memset(buf, '0', size);
    buf[size] = 0;
    return buf;
}

Block *proc_to_block_arg(Env *env, Value proc_or_nil) {
    if (proc_or_nil->is_nil()) {
        return nullptr;
    }
    return proc_or_nil->to_proc(env)->block();
}

#define NAT_SHELL_READ_BYTES 1024

Value shell_backticks(Env *env, Value command) {
    command->assert_type(env, Object::Type::String, "String");
    int pid;
    auto process = popen2(command->as_string()->c_str(), "r", pid);
    if (!process)
        env->raise_errno();
    char buf[NAT_SHELL_READ_BYTES];
    auto result = fgets(buf, NAT_SHELL_READ_BYTES, process);
    StringObject *out;
    if (result) {
        out = new StringObject { buf };
        while (1) {
            result = fgets(buf, NAT_SHELL_READ_BYTES, process);
            if (!result) break;
            out->append(env, buf);
        }
    } else {
        out = new StringObject {};
    }
    int status = pclose2(process, pid);
    set_status_object(env, pid, status);
    return out;
}

// https://stackoverflow.com/a/26852210/197498
FILE *popen2(const char *command, const char *type, int &pid) {
    pid_t child_pid;
    int fd[2];
    if (pipe(fd) != 0) {
        fprintf(stderr, "Unable to open pipe (errno=%d)", errno);
        abort();
    }

    const int read = 0;
    const int write = 1;

    bool is_read = strcmp(type, "r") == 0;

    if ((child_pid = fork()) == -1) {
        perror("fork");
        clean_up_and_exit(1);
    }

    // child process
    if (child_pid == 0) {
        if (is_read) {
            close(fd[read]); // close the READ end of the pipe since the child's fd is write-only
            dup2(fd[write], 1); // redirect stdout to pipe
        } else {
            close(fd[write]); // close the WRITE end of the pipe since the child's fd is read-only
            dup2(fd[read], 0); // redirect stdin to pipe
        }

        setpgid(child_pid, child_pid); // needed so negative PIDs can kill children of /bin/sh
        execl("/bin/sh", "/bin/sh", "-c", command, NULL);
        clean_up_and_exit(0);
    } else {
        if (is_read) {
            close(fd[write]); // close the WRITE end of the pipe since parent's fd is read-only
        } else {
            close(fd[read]); // close the READ end of the pipe since parent's fd is write-only
        }
    }

    pid = child_pid;

    if (is_read)
        return fdopen(fd[read], "r");

    return fdopen(fd[write], "w");
}

// https://stackoverflow.com/a/26852210/197498
int pclose2(FILE *fp, pid_t pid) {
    int stat;

    fclose(fp);
    while (waitpid(pid, &stat, 0) == -1) {
        if (errno != EINTR) {
            stat = -1;
            break;
        }
    }

    return stat;
}

void set_status_object(Env *env, int pid, int status) {
    auto status_obj = GlobalEnv::the()->Object()->const_fetch(SymbolObject::intern("Process"))->const_fetch(SymbolObject::intern("Status")).send(env, SymbolObject::intern("new"));
    status_obj->ivar_set(env, SymbolObject::intern("@to_i"), Value::integer(status));
    status_obj->ivar_set(env, SymbolObject::intern("@exitstatus"), Value::integer(WEXITSTATUS(status)));
    status_obj->ivar_set(env, SymbolObject::intern("@pid"), Value::integer(pid));
    env->global_set(SymbolObject::intern("$?"), status_obj);
}

const String *int_to_hex_string(nat_int_t num, bool capitalize) {
    if (num == 0) {
        return new String("0");
    } else {
        char buf[100]; // ought to be enough for anybody ;-)
        if (capitalize) {
            snprintf(buf, 100, "0X%llX", num);
        } else {
            snprintf(buf, 100, "0x%llx", num);
        }
        return new String(buf);
    }
}

Value super(Env *env, Value self, size_t argc, Value *args, Block *block) {
    auto current_method = env->current_method();
    auto super_method = self->klass()->find_method(env, SymbolObject::intern(current_method->name()), nullptr, current_method);
    if (!super_method)
        env->raise("NoMethodError", "super: no superclass method `{}' for {}", current_method->name(), self->inspect_str(env));
    assert(super_method != current_method);
    return super_method->call(env, self, argc, args, block);
}

void clean_up_and_exit(int status) {
    if (Heap::the().collect_all_at_exit())
        delete &Heap::the();
    exit(status);
}

}
