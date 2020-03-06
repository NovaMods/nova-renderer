# What does DXC do?

`int main`
* Calls `dxc::main`

`dxc::main`
* Inits their malloc
* Inits th arguments table
* Creates a `dxc::MainArgs` object
** Which in turn just copies the command-line arguments to an internal buffer
* Parses the options into a `dxc::DxcOpts` struct, using `ReadDxcOpts`
* Checks for errors, printing any if they exist
* Applies defaults for the shader entry point name
* Initializes a `dxc::DxcSupport` DLL object
* Creates a `dxc::DxcContext` with the options and support DLL object
* Preprocesses the file
* Compiles the fine
* Checks for and prints compilation errors

`dxc::DxcContext::Compile`
* Of lawdy
