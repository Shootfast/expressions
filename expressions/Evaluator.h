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
#include "llvm/IRBuilder.h"
#include "llvm/LLVMContext.h"
#include "llvm/Module.h"
#include "llvm/Intrinsics.h"
#include "llvm/PassManager.h"
#include "llvm/Analysis/Verifier.h"
#include "llvm/Analysis/Passes.h"
#include "llvm/DataLayout.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Support/Mutex.h"
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

		Evaluator(ASTNode *ast, VariableMap *map=NULL)
			: m_ast(ast)
			, m_map(map)
		{
		}

		T evaluate()
		{
			return evaluateSubtree(m_ast);
		}

	private:
		T evaluateSubtree(ASTNode* ast)
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
				if (!m_map)
				{
					throw EvaluatorException("Variable encountered but no VariableMap provided");
				}

				if (!m_map->count(variable))
				{
					std::ostringstream ss;
					ss << "No variable '" << variable << "' defined in VariableMap";
					throw Exception(ss.str().c_str());
				}
				return m_map->at(variable);
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


		ASTNode * m_ast;
		VariableMap *m_map;
};

#ifdef USE_LLVM

class LLVMEvaluator
{
	public:
		typedef std::map<std::string, float> VariableMap;

		LLVMEvaluator(ASTNode *ast, VariableMap *map=NULL)
			: m_module(new llvm::Module("expression jit", m_context))
			, m_builder(m_context)
			, m_engine(NULL)
			, m_fpm(m_module)
			, m_map(map)
			, m_function(NULL)
		{
			// Need to use a mutex here, because LLVM apparently isn't thread safe?
			mutex().acquire();

			llvm::InitializeNativeTarget();

			// Set up the JIT compiler
			std::string error;
			m_engine = llvm::EngineBuilder(m_module).setErrorStr(&error).create();
			if (!m_engine)
			{
				std::ostringstream ss;
				ss << "Could not initialize LLVM JIT, ";
				ss << error;
				mutex().release();
				throw EvaluatorException(ss.str().c_str());
			}

			
			// Set up the optimiser pipeline
			// Register how target lays out data structures
			m_fpm.add(new llvm::DataLayout(*m_engine->getDataLayout()));
			// Provide basic AliasAnalysis support for GVN
			m_fpm.add(llvm::createBasicAliasAnalysisPass());
			// Promote allocas to registers.
			m_fpm.add(llvm::createPromoteMemoryToRegisterPass());
			// Do simple "peephole" optimisations and bit-twiddling
			m_fpm.add(llvm::createInstructionCombiningPass());
			// Reassociate expressions
			m_fpm.add(llvm::createReassociatePass());
			// Eliminate common subexpressions
			m_fpm.add(llvm::createGVNPass());
			// Simplify the control flow graph
			m_fpm.add(llvm::createCFGSimplificationPass());

			m_fpm.doInitialization();
			

			// Create Function as entry point for LLVM
			std::vector<llvm::Type*> Void(0, llvm::Type::getFloatTy(m_context));
			llvm::FunctionType *FT = llvm::FunctionType::get(llvm::Type::getFloatTy(m_context), Void, false);
			m_function = llvm::Function::Create(FT, llvm::Function::ExternalLinkage, "", m_module);
			
			// Create block for code
			llvm::BasicBlock *BB = llvm::BasicBlock::Create(m_context, "entry", m_function);
			m_builder.SetInsertPoint(BB);

			// Convert AST to LLVM and place into function pointer
			try
			{
				m_builder.CreateRet(generateLLVM(ast));
			}
			catch (...)
			{
				mutex().release();
				throw;
			}

			// Verify that the function is well formed
			//( fails when intrinsics are used )
			//llvm::verifyFunction(*LF);

			// Dump the LLVM IR (for debugging)
			//m_module->dump();

			// Set the evaluate function call
			void *FPtr = m_engine->getPointerToFunction(m_function);
			evaluate = (float (*)()) (intptr_t)FPtr;

			mutex().release();
		}

		~LLVMEvaluator()
		{
			delete m_engine;
		}

		float (*evaluate)();
		
		float getVariable(const char *key)
		{
			if (!m_map)
			{
				throw EvaluatorException("Variable encountered, but no variable map provided");
			}

			if (m_map->count(key))
			{
				return m_map->at(key);
			}

			std::stringstream ss;
			ss << "Variable '" << key << "' not defined";
			throw EvaluatorException(ss.str().c_str());
		}


	private:

		// Convert AST into LLVM
		llvm::Value *generateLLVM(ASTNode* ast)
		{
			if(!ast)
			{
				throw EvaluatorException("No abstract syntax tree provided");
			}
			if(ast->type() == ASTNode::NUMBER)
			{
				NumberASTNode<float>* n = static_cast<NumberASTNode<float>* >(ast);
				return llvm::ConstantFP::get(m_context, llvm::APFloat(n->value()));
			}
			else if(ast->type() == ASTNode::VARIABLE)
			{
				VariableASTNode<float>* v = static_cast<VariableASTNode<float>* >(ast);
				std::string variable = v->variable();

				// Put the memory location of the variable from the map, into an LLVM constant
				llvm::Value *location = llvm::ConstantInt::get(llvm::Type::getIntNTy(m_context, sizeof(uintptr_t)*8), (uintptr_t) &m_map->at(variable));
				// Cast it to pointer
				llvm::Value *ptr = m_builder.CreateIntToPtr(location, llvm::Type::getFloatPtrTy(m_context));
				// Then dereference and load the pointer value
				llvm::Value *gep = m_builder.CreateGEP(ptr, llvm::ConstantInt::get(m_context, llvm::APInt(32, 0)), "geptmp");
				llvm::Value *load = m_builder.CreateLoad(gep, "loadtmp");

				return load;
			}
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
					case OperationASTNode::POW:
					{
						// Pow operator is defined in an intrinsic, so implement as a function call
						std::vector<llvm::Type*> arg_types(2, llvm::Type::getFloatTy(m_context)); // args are 2 floats
						return m_builder.CreateCall2(llvm::Intrinsic::getDeclaration(m_module, llvm::Intrinsic::pow, arg_types), v1, v2, "powtmp");
					}
					case OperationASTNode::MOD:   return m_builder.CreateFRem(v1, v2, "modtmp");
					default: throw EvaluatorException("Unknown operator in syntax tree");
				}
			}
			else if (ast->type() == ASTNode::FUNCTION1)
			{
				Function1ASTNode* f = static_cast<Function1ASTNode*>(ast);

				llvm::Value *v1 = generateLLVM(f->left());
				std::vector<llvm::Type*> arg_types(1, llvm::Type::getFloatTy(m_context)); // args are 1 float
				switch(f->function())
				{
					case Function1ASTNode::SIN:   return m_builder.CreateCall(llvm::Intrinsic::getDeclaration(m_module, llvm::Intrinsic::sin, arg_types), v1, "sintmp");
					case Function1ASTNode::COS:   return m_builder.CreateCall(llvm::Intrinsic::getDeclaration(m_module, llvm::Intrinsic::cos, arg_types), v1, "costmp");
					//case Function1ASTNode::TAN: return m_builder.CreateCall(llvm::Intrinsic::getDeclaration(m_module, llvm::Intrinsic::tan, arg_types), v1 ,"tantmp");
					case Function1ASTNode::TAN:
					{
						// No tan operator in LLVM 3.2. Have to use sin/cos
						llvm::Value *sin = m_builder.CreateCall(llvm::Intrinsic::getDeclaration(m_module, llvm::Intrinsic::sin, arg_types), v1, "sintmp");
						llvm::Value *cos = m_builder.CreateCall(llvm::Intrinsic::getDeclaration(m_module, llvm::Intrinsic::cos, arg_types), v1, "costmp");
						return m_builder.CreateFDiv(sin, cos, "tantmp");
					}
					case Function1ASTNode::SQRT:  return m_builder.CreateCall(llvm::Intrinsic::getDeclaration(m_module, llvm::Intrinsic::sqrt, arg_types), v1, "sqrttmp");
					case Function1ASTNode::LOG:   return m_builder.CreateCall(llvm::Intrinsic::getDeclaration(m_module, llvm::Intrinsic::log, arg_types), v1, "logtmp");
					case Function1ASTNode::LOG2:  return m_builder.CreateCall(llvm::Intrinsic::getDeclaration(m_module, llvm::Intrinsic::log2, arg_types), v1, "log2tmp");
					case Function1ASTNode::LOG10: return m_builder.CreateCall(llvm::Intrinsic::getDeclaration(m_module, llvm::Intrinsic::log10, arg_types), v1, "log10tmp");
					//case Function1ASTNode::CEIL:  return m_builder.CreateCall(llvm::Intrinsic::getDeclaration(m_module, llvm::Intrinsic::ceil, arg_types), v1, "ceiltmp");
					case Function1ASTNode::CEIL:
					{
						// No ceil operator in LLVM 3.2. Have to use floor +1
						llvm::Value *floor = m_builder.CreateCall(llvm::Intrinsic::getDeclaration(m_module, llvm::Intrinsic::floor, arg_types), v1, "floortmp");
						llvm::Value *one = llvm::ConstantFP::get(m_context, llvm::APFloat(1.0f));
						return m_builder.CreateFAdd(floor, one, "ceiltmp");
					}
					case Function1ASTNode::FLOOR: return m_builder.CreateCall(llvm::Intrinsic::getDeclaration(m_module, llvm::Intrinsic::floor, arg_types), v1, "floortmp");
					default: throw EvaluatorException("Unknown function in syntax tree");
				}
			}
			else if (ast->type() == ASTNode::FUNCTION2)
			{
				Function2ASTNode* f = static_cast<Function2ASTNode*>(ast);

				llvm::Value *v1 = generateLLVM(f->right()); // the operators are switched thanks to rpn notation
				llvm::Value *v2 = generateLLVM(f->left());
				switch(f->function())
				{
					case Function2ASTNode::MIN:  
					{
						// Min is 2 operations, Ordered Greater Than, then select
						llvm::Value *gt = m_builder.CreateFCmpOGT(v1, v2, "fogttmp");
						return m_builder.CreateSelect(gt, v2, v1, "mintmp");
					}
					case Function2ASTNode::MAX:
					{
						// Max is 2 operations, Ordered Greater Than, then select
						llvm::Value *gt = m_builder.CreateFCmpOGT(v1, v2, "fogttmp");
						return m_builder.CreateSelect(gt, v1, v2, "maxtmp");
					}
					case Function2ASTNode::POW: 
					{
						// Pow operator is defined in an intrinsic, so implement as a function call
						std::vector<llvm::Type*> arg_types(2, llvm::Type::getFloatTy(m_context)); // args are 2 floats
						return m_builder.CreateCall2(llvm::Intrinsic::getDeclaration(m_module, llvm::Intrinsic::pow, arg_types), v1, v2, "powtmp");
					}
					default: throw EvaluatorException("Unknown function in syntax tree");
				}
			}
			else if (ast->type() == ASTNode::COMPARISON)
			{
				ComparisonASTNode* c = static_cast<ComparisonASTNode*>(ast);
				llvm::Value *v1 = generateLLVM(c->right()); // the operators are switched thanks to rpn notation
				llvm::Value *v2 = generateLLVM(c->left());
				switch(c->comparison())
				{
					case ComparisonASTNode::EQUAL:              return m_builder.CreateFCmpOEQ(v1, v2, "foeqtmp");
					case ComparisonASTNode::NOT_EQUAL:          return m_builder.CreateFCmpONE(v1, v2, "fonetmp");
					case ComparisonASTNode::GREATER_THAN:       return m_builder.CreateFCmpOGT(v1, v2, "fogttmp");
					case ComparisonASTNode::GREATER_THAN_EQUAL: return m_builder.CreateFCmpOGE(v1, v2, "fogetmp");
					case ComparisonASTNode::LESS_THAN:          return m_builder.CreateFCmpOLT(v1, v2, "folttmp");
					case ComparisonASTNode::LESS_THAN_EQUAL:    return m_builder.CreateFCmpOLE(v1, v2, "foletmp");
					default: throw EvaluatorException("Unknown comparison in syntax tree");
				}
			}
			else if (ast->type() == ASTNode::LOGICAL)
			{
				LogicalASTNode *l = static_cast<LogicalASTNode*>(ast);
				llvm::Value *v1 = generateLLVM(l->right()); // the operators are switched thanks to rpn notation
				llvm::Value *v2 = generateLLVM(l->left());
				switch(l->operation())
				{
					case LogicalASTNode::AND: return m_builder.CreateAnd(v1, v2, "andtmp");
					case LogicalASTNode::OR:  return m_builder.CreateOr(v1, v2, "ortmp");
					default: throw EvaluatorException("Unknown logical operator in syntax tree");
				}
			}
			else if (ast->type() == ASTNode::BRANCH)
			{
				BranchASTNode* b = static_cast<BranchASTNode*>(ast);

				llvm::Value *cond = generateLLVM(b->condition());
				// If condition is a number, coerce it to bool
				if (cond->getType() == llvm::Type::getFloatTy(m_context))
				{
					cond = m_builder.CreateFCmpONE(cond, llvm::ConstantFP::get(m_context, llvm::APFloat(0.0f)), "ifcond");
				}
				llvm::Value *yes  = generateLLVM(b->yes());
				llvm::Value *no   = generateLLVM(b->no());

				llvm::Function *fun = m_builder.GetInsertBlock()->getParent();
				llvm::BasicBlock *thenBB  = llvm::BasicBlock::Create(m_context, "then", fun);
				llvm::BasicBlock *elseBB  = llvm::BasicBlock::Create(m_context, "else");
				llvm::BasicBlock *mergeBB = llvm::BasicBlock::Create(m_context, "ifcont");

				m_builder.CreateCondBr(cond, thenBB, elseBB);
				m_builder.SetInsertPoint(thenBB);
	
				m_builder.CreateBr(mergeBB);

				// emit else block
				thenBB = m_builder.GetInsertBlock();
				fun->getBasicBlockList().push_back(elseBB);
				m_builder.SetInsertPoint(elseBB);

				m_builder.CreateBr(mergeBB);
				elseBB = m_builder.GetInsertBlock();

				// emit the merge block
				fun->getBasicBlockList().push_back(mergeBB);
				m_builder.SetInsertPoint(mergeBB);
				llvm::PHINode *PN = m_builder.CreatePHI(llvm::Type::getFloatTy(m_context), 2, "iftmp");
				PN->addIncoming(yes, thenBB);
				PN->addIncoming(no, elseBB);
				return PN;
			}

			throw EvaluatorException("Incorrect syntax tree!");

		}

		static llvm::sys::SmartMutex<false>& mutex()
		{
			static llvm::sys::SmartMutex<false> m_mutex; return m_mutex;
		}
		

		llvm::LLVMContext m_context;
		llvm::Module *m_module;
		llvm::IRBuilder<> m_builder;
		llvm::ExecutionEngine *m_engine;
		llvm::FunctionPassManager m_fpm;
		llvm::Function *m_function;
		VariableMap *m_map;

};

#endif // USE_LLVM

} // namespace expr

#endif
