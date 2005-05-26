/*
 *	$Id$
 *	Copyright (c) 2005, IRIT UPS.
 *
 *	src/ets_WCETComputation.cpp -- WCETComputation class implementation.
 */

#include <otawa/ets/ETS.h>
#include <otawa/ast.h>
#include <otawa/ets/WCETComputation.h>
#include <elm/debug.h>

//#define WC_TRACE TRACE
#define WC_TRACE
//#define WC_OUT(txt) txt
#define WC_OUT(txt)


namespace otawa { namespace ets {

/**
 * @class WCETComputation
 * This processor is used for computing the WCET of the AST.
 */
 
 
/**
 * @fn WCETComputation::WCETComputation(ASTInfo *info);
 * Build the processor.
 * @param info All information about the current AST.
 */	
	
 
/**
 * Get the WCET of ast with the recursive function: WCETComputation::computation(AST *ast).
 * @param ast	AST to process.
 */	
void WCETComputation::processAST(AST *ast) {
	int tmp=computation(ast);
}


/**
 * @fn int WCETComputation::computation(AST *ast);
 * Compute the WCET of ast with the Timing Schema method by using annotations coming from other modules.
 * @param ast	AST to process.
 * @return	WCET of the current AST.
 * @exception	io::IOException if one number of iteration of loop or one WCET of function cannot be found.
 */
int WCETComputation::computation(AST *ast) {
		int ELSE, THEN, wcet, N;
		switch(ast->kind()) {
			case AST_Block:
				WC_OUT(cout << "|| " << ast->toBlock()->first()->get<String>(File::ID_Label,"problem! ") << " a pour wcet : " << ast->toBlock()->use<int>(ETS::ID_WCET)<< '\n');
				return ast->toBlock()->use<int>(ETS::ID_WCET);
				break;
			case AST_Seq:
				wcet=computation(ast->toSeq()->child1())
						+ computation(ast->toSeq()->child2());
				ast->toSeq()->set<int>(ETS::ID_WCET,wcet);
				WC_OUT(cout << "|| " << ast->toSeq()->first()->get<String>(File::ID_Label,"problem! ") << " a pour wcet : " << ast->toSeq()->use<int>(ETS::ID_WCET)<< '\n');
				return wcet;
				break;
			case AST_If:
				THEN=computation(ast->toIf()->condition())
					+ computation(ast->toIf()->thenPart());
				ELSE=computation(ast->toIf()->condition())
					+ computation(ast->toIf()->elsePart());
				if (THEN>ELSE) 
					ast->toIf()->set<int>(ETS::ID_WCET,THEN);
				else 
					ast->toIf()->set<int>(ETS::ID_WCET,ELSE);
				WC_OUT(cout << "|| " << ast->toIf()->condition()->first()->get<String>(File::ID_Label,"problem! ") << " a pour wcet : " << ast->toIf()->use<int>(ETS::ID_WCET)<< '\n');
				return ast->toIf()->use<int>(ETS::ID_WCET);
				break;
			case AST_While:
			 	N=ast->toWhile()->use<int>(ETS::ID_LOOP_COUNT);
			 	if (N == -1){
					WC_TRACE;
					throw io::IOException(String("Il manque le nb d'itérations du noeud : "+ ast->toWhile()->condition()->first()->get<String>(File::ID_Label, "problem! ")));
				}
				wcet=N*(computation(ast->toWhile()->condition())
							+ computation(ast->toWhile()->body()))
						+ computation(ast->toWhile()->condition());
				ast->toWhile()->set<int>(ETS::ID_WCET,wcet);
				WC_OUT(cout << "|| " << ast->toWhile()->condition()->first()->get<String>(File::ID_Label,"problem! ") << " a pour wcet : " << ast->toWhile()->use<int>(ETS::ID_WCET)<< '\n');		
				return wcet;
				break;
			case AST_DoWhile:
				N=ast->toDoWhile()->use<int>(ETS::ID_LOOP_COUNT);
				if (N == -1){
						WC_TRACE;
						throw io::IOException(String("Il manque le nb d'itération du noeud : "+ ast->toDoWhile()->condition()->first()->get<String>(File::ID_Label, "problem! ")));
				}
				wcet=N*(computation(ast->toDoWhile()->body())
							+ computation(ast->toDoWhile()->condition()));
				ast->toDoWhile()->set<int>(ETS::ID_WCET,wcet);	
				WC_OUT(cout << "|| " << ast->toDoWhile()->condition()->first()->get<String>(File::ID_Label,"problem! ") << " a pour wcet : " << ast->toDoWhile()->use<int>(ETS::ID_WCET)<< '\n');		
				return wcet;
				break;
			case AST_For:
				N=ast->toFor()->use<int>(ETS::ID_LOOP_COUNT);
				if (N == -1){
					WC_TRACE;
					throw io::IOException(String("Il manque le nb d'itération du noeud : "+ ast->toFor()->condition()->first()->get<String>(File::ID_Label, "problem! ")));
				}
				wcet=computation(ast->toFor()->initialization())
						+ N*(computation(ast->toFor()->condition())
							+ computation(ast->toFor()->incrementation())
							+ computation(ast->toFor()->body()))
						+ computation(ast->toFor()->condition());
				ast->toFor()->set<int>(ETS::ID_WCET,wcet);
				WC_OUT(cout << "|| " << ast->toFor()->condition()->first()->get<String>(File::ID_Label,"problem! ") << " a pour wcet : " << ast->toFor()->use<int>(ETS::ID_WCET)<< '\n');	
				return wcet;
				break;
			case AST_Call:{
				N=ast->toCall()->use<int>(ETS::ID_WCET);
				if (N == -1){
					Option< FunAST *> fun_res = ast_info->map().get(ast->toCall()->function()->name());
					if (fun_res){
						AST *fun_ast = (*fun_res)->ast();
						wcet=computation(fun_ast);
						ast->toCall()->set<int>(ETS::ID_WCET,wcet);
						WC_OUT(cout << "|| " << ast->toCall()->function()->name() << " a pour wcet : " << ast->toCall()->use<int>(ETS::ID_WCET)<< '\n');	
						return wcet;
					}
					else{
						WC_TRACE;
						throw io::IOException(String("Il manque le wcet de la fonction : "+ ast->toCall()->function()->name()));
					}
				}
				else
					return N;
				break;
			}
			default:
				return 0;
		}
}

} }// otawa::ets
