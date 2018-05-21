#include "lsp/common.h"
#include "lsp/all.h"
using namespace craft;
using namespace craft::lisp;

std::string s = R"f(
Content-Length: 83
Content-Type: "foo"

{
"id": null,
"result": null,
"code": -32601,
"message": "Method Not Implemented"
}
)f";

int main(int argc, char** argv) {
	craft::types::boot();
	
}