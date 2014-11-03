#include "ir_printer.h"

#include "ir.h"
#include "util.h"

using namespace std;

namespace simit {
namespace ir {

std::ostream &operator<<(std::ostream &os, const Func &function) {
  IRPrinter printer(os);
  printer.print(function);
  return os;
}

std::ostream &operator<<(std::ostream &os, const Expr &expr) {
  IRPrinter printer(os);
  printer.print(expr);
  return os;
}

std::ostream &operator<<(std::ostream &os, const Stmt &Stmt) {
  IRPrinter printer(os);
  printer.print(Stmt);
  return os;
}

std::ostream &operator<<(std::ostream &os, const IRNode &node) {
  IRPrinter printer(os);
  printer.print(node);
  return os;
}


// class IRPrinter
IRPrinter::IRPrinter(std::ostream &os, signed indent) : os(os), indentation(0) {
}

void IRPrinter::print(const Func &func) {
  if (func.defined()) {
    func.accept(this);
  }
  else {
    os << "Func()";
  }
}

void IRPrinter::print(const Expr &expr) {
  if (expr.defined()) {
    expr.accept(this);
  }
  else {
    os << "Expr()";
  }
}

void IRPrinter::print(const Stmt &stmt) {
  if (stmt.defined()) {
    stmt.accept(this);
  }
  else {
    os << "Stmt()";
  }
}

void IRPrinter::print(const IRNode &node) {
  node.accept(this);
}

void IRPrinter::visit(const Literal *op) {
  // TODO: Fix value printing to print matrices and tensors properly
  switch (op->type.kind()) {
    case Type::Tensor: {
      size_t size;
      ScalarType::Kind componentType;

      assert(op->type.kind() == Type::Tensor);
      const TensorType *type = op->type.toTensor();
      size = type->size();
      componentType = type->componentType.kind;

      switch (componentType) {
        case ScalarType::Int: { {
          const int *idata = static_cast<const int*>(op->data);
          if (size == 1) {
            os << idata[0];
          }
          else {
            os << "[" << idata[0];
            for (size_t i=0; i < size; ++i) {
              os << ", " << idata[i];
            }
            os << "]";
          }
          break;
        }
        case ScalarType::Float: {
          const double *fdata = static_cast<const double*>(op->data);
          if (size == 1) {
            os << fdata[0];
          }
          else {
            os << "[" << to_string(fdata[0]);
            for (size_t i=1; i < size; ++i) {
              os << ", " + to_string(fdata[i]);
            }
            os << "]";
          }
          break;
        }
        }
      }
      break;
    }
    case Type::Element:
      NOT_SUPPORTED_YET;
    case Type::Set:
      NOT_SUPPORTED_YET;
      break;
    case Type::Tuple:
      NOT_SUPPORTED_YET;
      break;
  }
}

void IRPrinter::visit(const VarExpr *op) {
  os << op->var;
}

void IRPrinter::visit(const Result *) {
  os << "result";
}

void IRPrinter::visit(const FieldRead *op) {
  print(op->elementOrSet);
  os << "." << op->fieldName;
}

void IRPrinter::visit(const TensorRead *op) {
  print(op->tensor);
  os << "(";
  auto indices = op->indices;
  if (indices.size() > 0) {
    print(indices[0]);
  }
  for (size_t i=1; i < indices.size(); ++i) {
    os << ",";
    print(indices[i]);
  }
  os << ")";
}

void IRPrinter::visit(const TupleRead *op) {
  print(op->tuple);
  os << "(";
  print(op->index);
  os << ")";
}

void IRPrinter::visit(const IndexRead *op) {
  print(op->edgeSet);
  os << "." << op->indexName;
}

void IRPrinter::visit(const Length *op) {
  os << op->indexSet;
}

void IRPrinter::visit(const Load *op) {
  print(op->buffer);
  os << "[";
  print(op->index);
  os << "]";
}

void IRPrinter::visit(const IndexedTensor *op) {
  print(op->tensor);
  os << "{" << util::join(op->indexVars,",") << "}";
}

void IRPrinter::visit(const IndexExpr *op) {
  os << "({" + simit::util::join(op->resultVars, ",") + "} ";
  print(op->value);
  os << ")";
}

void IRPrinter::visit(const Call *op) {
  os << op->func.getName() << "(" << util::join(op->func.getArguments()) << ")";
}

void IRPrinter::visit(const Neg *op) {
  os << "-";
  print(op->a);
}

void IRPrinter::visit(const Add *op) {
  os << "(";
  print(op->a);
  os << " + ";
  print(op->b);
  os << ")";
}

void IRPrinter::visit(const Sub *op) {
  os << "(";
  print(op->a);
  os << " - ";
  print(op->b);
  os << ")";
}

void IRPrinter::visit(const Mul *op) {
  os << "(";
  print(op->a);
  os << " * ";
  print(op->b);
  os << ")";
}

void IRPrinter::visit(const Div *op) {
  os << "(";
  print(op->a);
  os << " / ";
  print(op->b);
  os << ")";
}

void IRPrinter::visit(const AssignStmt *op) {
  indent();
  os << op->var << " = ";
  print(op->value);
  os << ";";
}

void IRPrinter::visit(const Map *op) {
  indent();
  os << util::join(op->vars) << " = ";
  os << "map " << op->function.getName();
  os << " to ";
  print(op->target);
  if (op->neighbors.defined()) {
    os << " with ";
    print(op->neighbors);
  }
  if (op->reduction.getKind() != ReductionOperator::Undefined) {
    os << " reduce " << op->reduction;
  }
  os << ";";
}

void IRPrinter::visit(const FieldWrite *op) {
  indent();
  print(op->elementOrSet);
  os << "." << op->fieldName << " = ";
  print(op->value);
  os << ";";
}

void IRPrinter::visit(const TensorWrite *op) {
  indent();
  print(op->tensor);
  os << "(";
  auto indices = op->indices;
  if (indices.size() > 0) {
    print(indices[0]);
  }
  for (size_t i=1; i < indices.size(); ++i) {
    os << ",";
    print(indices[i]);
  }
  os << ") = ";
  print(op->value);
  os << ";";
}

void IRPrinter::visit(const Store *op) {
  indent();
  print(op->buffer);
  os << "[";
  print(op->index);
  os << "] = ";
  print(op->value);
  os << ";";
}

void IRPrinter::visit(const For *op) {
  indent();
  os << "for " << op->var << " in ";
  switch (op->domain.kind) {
    case ForDomain::IndexSet:
      os << op->domain.indexSet;
      break;
    case ForDomain::Endpoints:
      os << op->domain.set << ".endpoints[" << op->domain.var << "]";
      break;
    case ForDomain::Edges:
      os << op->domain.set << ".edges[" << op->domain.var << "]";
      break;
  }

  os << ":" << endl;
  ++indentation;
  print(op->body);
  --indentation;
}

void IRPrinter::visit(const IfThenElse *op) {
  indent();
  os << "ifthenelse;";
}

void IRPrinter::visit(const Block *op) {
  print(op->first);
  if (op->rest.defined()) {
    os << endl;
    print(op->rest);
  }
}

void IRPrinter::visit(const Pass *op) {
  indent();
  os << "pass;";
}

void IRPrinter::visit(const Func *func) {
  os << "func " << func->getName() << "(";
  if (func->getArguments().size() > 0) {
    const Var &arg = func->getArguments()[0];
    os << arg << " : " << arg.type;
  }
  for (size_t i=1; i < func->getArguments().size(); ++i) {
    const Var &arg = func->getArguments()[i];
    os << ", " << arg << " : " << arg.type;
  }
  os << ")";

  if (func->getResults().size() > 0) {
    os << " -> (";
    const Var &res = func->getResults()[0];
    os << res << " : " << res.type;

    for (size_t i=1; i < func->getResults().size(); ++i) {
      const Var &res = func->getResults()[i];
      os << ", " << res << " : " << res.type;
    }
    os << ")";
  }

  if (func->getBody().defined()) {
    os << ":" << endl;

    ++indentation;
    print(func->getBody());
    --indentation;
  }
  else {
    os << ";";
  }
}

void IRPrinter::indent() {
  for (unsigned i=0; i<indentation; ++i) {
    os << "  ";
  }
}

}} //namespace simit::ir