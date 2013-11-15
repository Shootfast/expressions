#ifndef EVALUATOR_H
#define EVALUATOR_H


#include "math.h"
#include <algorithm>

#include "AST.h"
#include "Exception.h"

#ifdef USE_LLVM

#include "llvm/DerivedTypes.h"
#include "llvm/ExecutionEngine/ExecutionEngine.h"
#include "llvm/ExecutionEngine/JIT.h"
#include "llvm/LLVMContext.h"
#include "llvm/Module.h"
#include "llvm/PassManager.h"
#include "llvm/Analysis/Verifier.h"
#include "llvm/Analysis/Passes.h"
#include "llvm/Target/TargetData.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Support/IRBuilder.h"
#include "llvm/Support/TargetSelect.h"

#endif //USE_LLVM

namespace expr
{

class EvaluatorException : public Exception
{
    public:
       EvaluatorException(const char * message)
        : Exception(message)
       {
       }
};



template <typename T>
class Evaluator
{
	public:
		typedef std::map<std::string, T> VariableMap;

		T evaluate(ASTNode* ast, VariableMap *map=NULL)
		{
			if(ast == NULL)
			{
				throw EvaluatorException("Incorrect abstract syntax tree");
			}

			return evaluateSubtree(ast, map);
		}

	private:
		T evaluateSubtree(ASTNode* ast, VariableMap *map)
		{
			if(!ast)
			{
				throw EvaluatorException("No abstract syntax tree provided");
			}
			if(ast->type() == ASTNode::NUMBER)
			{
				NumberASTNode<T>* n = static_cast<NumberASTNode<T>* >(ast);
				return n->value();
			}
			else if(ast->type() == ASTNode::VARIABLE)
			{
				VariableASTNode<T>* v = static_cast<VariableASTNode<T>* >(ast);
				std::string variable = v->variable();
				if (!map)
				{
					throw EvaluatorException("Variable encountered but no VariableMap provided");
				}

				if (!map->count(variable))
				{
					std::ostringstream ss;
					ss << "No variable '" << variable << "' defined in VariableMap";
					throw Exception(ss.str().c_str());
				}
				return map->at(variable);
			}
			else if (ast->type() == ASTNode::OPERATION)
			{
				OperationASTNode* op = static_cast<OperationASTNode*>(ast);

				T v1 = evaluateSubtree(op->right()); // the operators are switched thanks to rpn notation
				T v2 = evaluateSubtree(op->left());
				switch(op->operation())
				{
					case OperationASTNode::PLUS:  return v1 + v2;
					case OperationASTNode::MINUS: return v1 - v2;
					case OperationASTNode::MUL:   return v1 * v2;
					case OperationASTNode::DIV:   return v1 / v2;
					case OperationASTNode::POW:   return (T) pow(v1,v2);
					case OperationASTNode::MOD:   return (T) fmod(v1,v2);
					default: throw EvaluatorException("Unknown operator in syntax tree");
				}
			}
			else if (ast->type() == ASTNode::FUNCTION1)
			{
				Function1ASTNode* f = static_cast<Function1ASTNode*>(ast);

				T v1 = evaluateSubtree(f->left());
				switch(f->function())
				{
					case Function1ASTNode::SIN:   return (T) sin(v1);
					case Function1ASTNode::COS:   return (T) cos(v1);
					case Function1ASTNode::TAN:   return (T) tan(v1);
					case Function1ASTNode::SQRT:  return (T) sqrt(v1);
					case Function1ASTNode::LOG:   return (T) log(v1);
					case Function1ASTNode::LOG2:  return (T) log2(v1);
					case Function1ASTNode::LOG10: return (T) log10(v1);
					case Function1ASTNode::CEIL:  return (T) ceil(v1);
					case Function1ASTNode::FLOOR: return (T) floor(v1);
					default: throw EvaluatorException("Unknown function in syntax tree");
				}
			}
			else if (ast->type() == ASTNode::FUNCTION2)
			{
				Function2ASTNode* f = static_cast<Function2ASTNode*>(ast);

				T v1 = evaluateSubtree(f->right()); // the operators are switched thanks to rpn notation
				T v2 = evaluateSubtree(f->left());
				switch(f->function())
				{
					case Function2ASTNode::MIN:  return std::min(v1, v2);
					case Function2ASTNode::MAX:  return std::max(v1, v2);
					case Function2ASTNode::POW:  return (T) pow(v1, v2);
					default: throw EvaluatorException("Unknown function in syntax tree");
				}
			}
			else if (ast->type() == ASTNode::COMPARISON)
			{
				ComparisonASTNode* c = static_cast<ComparisonASTNode*>(ast);
				T v1 = evaluateSubtree(c->right()); // the operators are switched thanks to rpn notation
				T v2 = evaluateSubtree(c->left());
				switch(c->comparison())
				{
					case ComparisonASTNode::EQUAL:              return v1 == v2;
					case ComparisonASTNode::NOT_EQUAL:          return v1 != v2;
					case ComparisonASTNode::GREATER_THAN:       return v1 >  v2;
					case ComparisonASTNode::GREATER_THAN_EQUAL: return v1 >= v2;
					case ComparisonASTNode::LESS_THAN:          return v1 <  v2;
					case ComparisonASTNode::LESS_THAN_EQUAL:    return v1 <= v2;
					default: throw EvaluatorException("Unknown comparison in syntax tree");
				}
			}
			else if (ast->type() == ASTNode::LOGICAL)
			{
				LogicalASTNode *l = static_cast<LogicalASTNode*>(ast);
				T v1 = evaluateSubtree(l->right()); // the operators are switched thanks to rpn notation
				T v2 = evaluateSubtree(l->left());
				switch(l->operation())
				{
					case LogicalASTNode::AND: return v1 && v2;
					case LogicalASTNode::OR:  return v1 || v2;
					default: throw EvaluatorException("Unknown logical operator in syntax tree");
				}
			}
			else if (ast->type() == ASTNode::BRANCH)
			{
				BranchASTNode* b = static_cast<BranchASTNode*>(ast);
				if ((bool)evaluateSubtree(b->condition()) == true)
				{
					return evaluateSubtree(b->yes());
				}
				else
				{
					return evaluateSubtree(b->no());
				}
			}

			throw EvaluatorException("Incorrect syntax tree!");
		}
};

#ifdef USE_LLVM

template <typename T>
class LLVMEvaluator
{
	public:
		typedef std::map<std::string, T> VariableMap;

		LLVMEvaluator(ASTNode *ast)
			: m_context(llvm::getGlobalContext())
			, m_module(new llvm::Module("expression jit", m_context))
			, m_builder(m_context)
			, m_engine(NULL)
			, m_fpm(m_module)
		{
			if(ast == NULL)
			{
				throw EvaluatorException("Incorrect abstract syntax tree");
			}

			// Set up the JIT compiler
			std::string error;
			m_engine = llvm::EngineBuilder(m_module).setErrorStr(&error).create();
			if (!m_engine)
			{
				std::ostringstream ss;
				ss << "Could not initialize LLVM JIT, ";
				ss << error;
				throw EvaluatorException(ss.str().c_str());
			}

			// Set up the optimiser pipeline
			// Register how target lays out data structures
			m_fpm.add(new llvm::TargetData(*m_engine->getTargetData()));
			// Provide basic AliasAnalysis support for GVN
			m_fpm.add(llvm::createBasicAliasAnalysisPass());
			// Do simple "peephole" optimisations and bit-twiddling
			m_fpm.add(llvm::createInstructionCombiningPass());
			// Reassociate expressions
			m_fpm.add(llvm::createReassociatePass());
			// Eliminate common subexpressions
			m_fpm.add(llvm::createGVNPass());
			// Simplify the control flow graph
			m_fpm.add(llvm::createCFGSimplificationPass());

			m_fpm.doInitialization();


			// Get function from LLVM
			std::vector<llvm::Type*> Doubles(0, Type::getDoubleTy(m_context));
			llvm::FunctionType *FT = FunctionType::get(Type::getDoubleTy(m_context),Doubles, false);
			llvm::Function *LF = Function::Create(FT, Function::ExternalLinkage, "go", m_module);
generateLLVM(ast);
	
			LF->dump();

			void *FPtr = m_engine->getPointerToFunction(LF);
			double (*FP)() = (double (*)()) (intptr_t)FPtr;

			std::cout << "result: " << FP() << std::endl;
		}

	private:

		llvm::Value *generateLLVM(ASTNode* ast)
		{
			if(!ast)
			{
				throw EvaluatorException("No abstract syntax tree provided");
			}
			if(ast->type() == ASTNode::NUMBER)
			{
				NumberASTNode<T>* n = static_cast<NumberASTNode<T>* >(ast);
				return llvm::ConstantFP::get(llvm::getGlobalContext(), llvm::APFloat(n->value()));
			}
			/*else if(ast->type() == ASTNode::VARIABLE)
			{
				//TODO investigate this
				VariableASTNode<T>* v = static_cast<VariableASTNode<T>* >(ast);
				std::string variable = v->variable();
				if (!map)
				{
					throw EvaluatorException("Variable encountered but no VariableMap provided");
				}

				if (!map->count(variable))
				{
					std::ostringstream ss;
					ss << "No variable '" << variable << "' defined in VariableMap";
					throw Exception(ss.str().c_str());
				}
				return map->at(variable);
			}*/
			else if (ast->type() == ASTNode::OPERATION)
			{
				OperationASTNode* op = static_cast<OperationASTNode*>(ast);

				llvm::Value *v1 = generateLLVM(op->right()); // the operators are switched thanks to rpn notation
				llvm::Value *v2 = generateLLVM(op->left());
				switch(op->operation())
				{
					case OperationASTNode::PLUS:  return m_builder.CreateFAdd(v1, v2, "addtmp");
					case OperationASTNode::MINUS: return m_builder.CreateFSub(v1, v2, "subtmp");
					case OperationASTNode::MUL:   return m_builder.CreateFMul(v1, v2, "multmp");
					case OperationASTNode::DIV:   return m_builder.CreateFDiv(v1, v2, "divtmp");
				//	case OperationASTNode::POW:   return (T) pow(v1,v2);
					case OperationASTNode::MOD:   return m_builder.CreateFRem(v1, v2, "modtmp");
					default: throw EvaluatorException("Unknown operator in syntax tree");
				}
			}/*
			else if (ast->type() == ASTNode::FUNCTION1)
			{
				Function1ASTNode* f = static_cast<Function1ASTNode*>(ast);

				T v1 = evaluateSubtree(f->left());
				switch(f->function())
				{
					case Function1ASTNode::SIN:   return (T) sin(v1);
					case Function1ASTNode::COS:   return (T) cos(v1);
					case Function1ASTNode::TAN:   return (T) tan(v1);
					case Function1ASTNode::SQRT:  return (T) sqrt(v1);
					case Function1ASTNode::LOG:   return (T) log(v1);
					case Function1ASTNode::LOG2:  return (T) log2(v1);
					case Function1ASTNode::LOG10: return (T) log10(v1);
					case Function1ASTNode::CEIL:  return (T) ceil(v1);
					case Function1ASTNode::FLOOR: return (T) floor(v1);
					default: throw EvaluatorException("Unknown function in syntax tree");
				}
			}
			else if (ast->type() == ASTNode::FUNCTION2)
			{
				Function2ASTNode* f = static_cast<Function2ASTNode*>(ast);

				T v1 = evaluateSubtree(f->right()); // the operators are switched thanks to rpn notation
				T v2 = evaluateSubtree(f->left());
				switch(f->function())
				{
					case Function2ASTNode::MIN:  return std::min(v1, v2);
					case Function2ASTNode::MAX:  return std::max(v1, v2);
					case Function2ASTNode::POW:  return (T) pow(v1, v2);
					default: throw EvaluatorException("Unknown function in syntax tree");
				}
			}
			else if (ast->type() == ASTNode::COMPARISON)
			{
				ComparisonASTNode* c = static_cast<ComparisonASTNode*>(ast);
				T v1 = evaluateSubtree(c->right()); // the operators are switched thanks to rpn notation
				T v2 = evaluateSubtree(c->left());
				switch(c->comparison())
				{
					case ComparisonASTNode::EQUAL:              return v1 == v2;
					case ComparisonASTNode::NOT_EQUAL:          return v1 != v2;
					case ComparisonASTNode::GREATER_THAN:       return v1 >  v2;
					case ComparisonASTNode::GREATER_THAN_EQUAL: return v1 >= v2;
					case ComparisonASTNode::LESS_THAN:          return v1 <  v2;
					case ComparisonASTNode::LESS_THAN_EQUAL:    return v1 <= v2;
					default: throw EvaluatorException("Unknown comparison in syntax tree");
				}
			}
			else if (ast->type() == ASTNode::LOGICAL)
			{
				LogicalASTNode *l = static_cast<LogicalASTNode*>(ast);
				T v1 = evaluateSubtree(l->right()); // the operators are switched thanks to rpn notation
				T v2 = evaluateSubtree(l->left());
				switch(l->operation())
				{
					case LogicalASTNode::AND: return v1 && v2;
					case LogicalASTNode::OR:  return v1 || v2;
					default: throw EvaluatorException("Unknown logical operator in syntax tree");
				}
			}
			else if (ast->type() == ASTNode::BRANCH)
			{
				BranchASTNode* b = static_cast<BranchASTNode*>(ast);
				if ((bool)evaluateSubtree(b->condition()) == true)
				{
					return evaluateSubtree(b->yes());
				}
				else
				{
					return evaluateSubtree(b->no());
				}
			}
			*/
			throw EvaluatorException("Incorrect syntax tree!");

		}
		

		llvm::LLVMContext &m_context;
		llvm::Module *m_module;
		llvm::IRBuilder<> m_builder;
		llvm::ExecutionEngine *m_engine;
		llvm::FunctionPassManager m_fpm;
		std::map<std::string, llvm::AllocaInst*> m_map;

};
#endif // USE_LLVM

} // namespace expr

#endif
