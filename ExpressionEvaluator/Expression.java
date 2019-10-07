package apps;

import java.io.*;

import java.util.*;
import java.util.regex.*;

import structures.Stack;

public class Expression {

	/**
	 * Expression to be evaluated
	 */
	String expr;                
    
	/**
	 * Scalar symbols in the expression 
	 */
	ArrayList<ScalarSymbol> scalars;   
	
	/**
	 * Array symbols in the expression
	 */
	ArrayList<ArraySymbol> arrays;
    
    /**
     * String containing all delimiters (characters other than variables and constants), 
     * to be used with StringTokenizer
     */
    public static final String delims = " \t*+-/()[]";
    
    /**
     * Initializes this Expression object with an input expression. Sets all other
     * fields to null.
     * 
     * @param expr Expression
     */
    public Expression(String expr) {
        this.expr = expr;
//        this.scalars = new ArrayList<>();
//        this.arrays = new ArrayList<>();
    }

    /**
     * Populates the scalars and arrays lists with symbols for scalar and array
     * variables in the expression. For every variable, a SINGLE symbol is created and stored,
     * even if it appears more than once in the expression.
     * At this time, values for all variables are set to
     * zero - they will be loaded from a file in the loadSymbolValues method.
     */
    public void buildSymbols() {
    		scalars = new ArrayList<>();
    		arrays = new ArrayList<>();
//    	ScalarSymbol test = new ScalarSymbol("test");
//    	System.out.println(test);
    		for (int i = 0; i<expr.length(); i++) {
    			if (Character.isLetter(expr.charAt(i))) {
    				String s = "";
    				boolean isArray = false;
    				while (i<expr.length() && Character.isLetter(expr.charAt(i))) {
    					char x = expr.charAt(i);
    					String f = Character.toString(x);
    					s = s+f;
    					i++;
    				}
    				if (i<expr.length()-1 && expr.charAt(i) == '[') {
    					isArray = true;
    				}
    				if (isArray==true) {
    					boolean exists = false;
    					for (int j =0; j<arrays.size();j++) {
    						if (s.equals(arrays.get(j).name)) {
    							exists = true;
    						}
    					}
    					if (exists == false) {
    						ArraySymbol array1 = new ArraySymbol(s);
        					this.arrays.add(array1);
    					}
    					
    				}else {
    					boolean exists = false;
    					for (int j =0; j<scalars.size();j++) {
    						if (s.equals(scalars.get(j).name)) {
    							exists = true;
    						}
    					}
    					if (exists == false) {
    						ScalarSymbol scalar1 = new ScalarSymbol(s);
        					this.scalars.add(scalar1);
    					}
    					
    				}
    			}
    			//System.out.println(arrays.size());
    		}
    }
    
    /**
     * Loads values for symbols in the expression
     * 
     * @param sc Scanner for values input
     * @throws IOException If there is a problem with the input 
     */
    public void loadSymbolValues(Scanner sc) 
    throws IOException {
        while (sc.hasNextLine()) {
            StringTokenizer st = new StringTokenizer(sc.nextLine().trim());
            int numTokens = st.countTokens();
            String sym = st.nextToken();
            ScalarSymbol ssymbol = new ScalarSymbol(sym);
            ArraySymbol asymbol = new ArraySymbol(sym);
            int ssi = scalars.indexOf(ssymbol);
            int asi = arrays.indexOf(asymbol);
            if (ssi == -1 && asi == -1) {
            	continue;
            }
            int num = Integer.parseInt(st.nextToken());
            if (numTokens == 2) { // scalar symbol
                scalars.get(ssi).value = num;
            } else { // array symbol
            	asymbol = arrays.get(asi);
            	asymbol.values = new int[num];
                // following are (index,val) pairs
                while (st.hasMoreTokens()) {
                    String tok = st.nextToken();
                    StringTokenizer stt = new StringTokenizer(tok," (,)");
                    int index = Integer.parseInt(stt.nextToken());
                    int val = Integer.parseInt(stt.nextToken());
                    asymbol.values[index] = val;              
                }
            }
        }
    }
    
    
    /**
     * Evaluates the expression, using RECURSION to evaluate subexpressions and to evaluate array 
     * subscript expressions.
     * 
     * @return Result of evaluation
     */
    public float evaluate() {
    		//the below tokenizer creates tokens for the delims too
    		for (int i = 0; i<this.scalars.size();i++) {
			if (expr.contains(this.scalars.get(i).name)) {
			//	System.out.println("true");
				String value = Integer.toString(this.scalars.get(i).value);
			//	System.out.println(this.scalars.get(i).name);
			//	System.out.println(value);
				expr = expr.replaceAll(this.scalars.get(i).name,value);
			}
		}
 //   		System.out.println("expression: "+expr);
    		StringTokenizer st = new StringTokenizer(expr,delims,true);
    		ArrayList<String> elements = new ArrayList<String>();
    		while (st.hasMoreElements()) {
    			String element = st.nextToken();
 //   			System.out.println("nextToken: "+element);
    			if (element == " " || element == "/t") {
    				continue;
    			} else{
    				elements.add(element);
    			}
    		}
//    		System.out.println("elements: "+elements);
    		
    		String [] tokens = new String[elements.size()];
    		for (int i=0;i<elements.size();i++) {
    			tokens[i] = elements.get(i);
    		}
 
//    		System.out.println("tokens:");
//    		printStringArrays(tokens,0,tokens.length-1);

    		return evaluate(tokens,0,elements.size()-1);
    }
    
    
    private float evaluate(String[] splits, int beginning, int ending) {
    		Stack<Float> numbers = new Stack<Float>();
    		Stack<String> operators = new Stack<String>();

//    		printStringArrays(splits, beginning, ending);
    		

    		for (int i = beginning;i<=ending;i++) {
//    			System.out.println(i);
    			//System.out.println(splits[i]);
    			if (splits[i].equals("(")) {
    				int rightParenIndex = 0;
    				int leftParen = 1;
    				for (int j = i+1;j<=ending;j++) {
    					if (splits[j].equals("(")) {
    						leftParen++;
    					} else if(splits[j].equals(")")){
    						leftParen--;
    						if (leftParen == 0) {
    							rightParenIndex = j;
    							break;
    						}
    					} else {
    						continue;
    					}
    				}
    				
    				
    				numbers.push(evaluate(splits,i+1,rightParenIndex-1));
//    				System.out.println("number: "+numbers.peek());
    				i = rightParenIndex;
// 				System.out.println("rightparen: "+rightParenIndex);
    			} else if(splits[i].equals("[")) {
 //   				System.out.println("brackes is god");
    				String token = splits[i-1];
    				int rightBracketInd = 0;
    				int leftBracket = 1;
    				for (int j = i+1; j<=ending; j++) {
 //   					System.out.println("j: "+j);
    					if (splits[j].equals("[")) {
    						leftBracket++;
    					}else if(splits[j].equals("]")) {
    						leftBracket--;
    						if (leftBracket == 0) {
//    							System.out.println("brackets closed");
    							rightBracketInd = j;
    							break;
    						} 
    					} else {
    						continue;
    					}
    				}
 //   				System.out.println("left:" + (i+1) + "||right: "+(rightBracketInd-1) );
    				int index = (int) evaluate(splits,i+1,rightBracketInd-1);
//    				System.out.println("floats: " + Float.valueOf(this.arrayValue(token,index)));
    				numbers.push(Float.valueOf(this.arrayValue(token,index)));
    				i = rightBracketInd;
 //   				System.out.println("bracketrue");
    			} else if(splits[i].matches("[0123456789]+")) {
    				numbers.push(Float.parseFloat(splits[i]));
 //   				System.out.println("number: "+Float.parseFloat(splits[i]));
 //   				System.out.println("numtrue");
    			} else if (splits[i].equals("+") || splits[i].equals("-") 
    					|| splits[i].equals("*") || splits[i].equals("/")) {
   	    		
    				while (operators.isEmpty() == false && (operators.peek().equals("*") 
    						|| operators.peek().equals("/"))) {
    					
    					String operator = operators.pop();
    					float number2 = numbers.pop();
    					float number1 = numbers.pop();
//    					System.out.println("number to push: "+calculate(number1,number2,operator));
    					numbers.push(calculate(number1,number2,operator));
 //   					System.out.println("true");
    				}
    				
    				operators.push(splits[i]);

    			} else {
  //  				System.out.println("is letters");
    				continue;
    			}
    		}
  		
//    		System.out.println("numbers in stack:");
//    		numbers.dumpStack();
    		if (operators.isEmpty() == false) {
//    			System.out.println("ick");
//    			System.out.println("numbers: ");
//    			numbers.dumpStack();
    			Stack<Float> numbers2 = new Stack<Float>();
        		Stack<String> operators2 = new Stack<String>();
        		while (operators.isEmpty() == false) {
        			String tempop = operators.pop();
        			operators2.push(tempop);
        		}
        		
//        		System.out.println("operators2: ");
//        		operators2.dumpStack();
        		
//        		System.out.println("Dumping Ops2 Stack");
//        		operators2.dumpStack();
        		
        		while (numbers.isEmpty() == false) {
        			float tempnum = numbers.pop();
  //      			System.out.println("tempnum: "+tempnum);
        			numbers2.push(tempnum);
        		}
//        		System.out.println("numbers2: ");
//        		numbers2.dumpStack();
        		
//        		System.out.println("Dumping Num2 Stack");
//        		numbers2.dumpStack();
        		
        		/** started new thing**/
        		Stack<Float> numbers3 = new Stack<Float>();
        		Stack<String> operators3 = new Stack<String>();
        		
//        		numbers2.dumpStack();
//        		operators2.dumpStack();
        	
        		
        		numbers3.push(numbers2.pop());
//        		System.out.println("operators2 size: " + operators2.size());
        		int j = operators2.size();
        		
        		for (int i = 0; i<=j; i++) {
      //  			System.out.println("i: "+i);
        			if (operators2.size() != 0) {
        				if (operators2.peek().equals("*") || operators2.peek().equals("/")) {
            				numbers3.push(calculate(numbers3.pop(),numbers2.pop(),operators2.pop()));
            			} else {
            				operators3.push(operators2.pop());
            				numbers3.push(numbers2.pop());
            			}
//            			System.out.println("round " + i + ":");
//            			numbers3.dumpStack();
        			} else {
        				break;
        			}
        			
        		
        		}
//        		System.out.println("numbers3: ");
//        		numbers3.dumpStack();
//        		System.out.println("operators3: ");
//        		operators3.dumpStack();

        		if (operators3.isEmpty() == false) {
  //      			System.out.println("ick2");
        			Stack<Float> numbers4 = new Stack<Float>();
            		Stack<String> operators4 = new Stack<String>();
            		while (operators3.isEmpty() == false) {
            			String tempop = operators3.pop();
            			operators4.push(tempop);
            		}
            		while (numbers3.isEmpty() == false) {
            			float tempnum = numbers3.pop();
 //           			System.out.println("tempnum: "+tempnum);
            			numbers4.push(tempnum);
            		}
            		while (operators4.isEmpty() == false) {
            			String operator4 = operators4.pop();
            			float number1 = numbers4.pop();
            			float number2 = numbers4.pop();
//            			System.out.println("num1 "+number1+" num2 "+number2+" operator2 "+operator4);
            			numbers4.push(calculate(number1,number2,operator4));
            		}
            		return numbers4.pop();
            		
            		
        		} else {
        			return numbers3.pop();
        		}
        		
        		/** end new thing **/
    		} else {
    			return numbers.pop();
    		}
    }

   /**break**/ 
    private float calculate (float num1, float num2, String operand) {
    		switch(operand){
    		case "+":
    			return num1+num2;
    		case "-":
    			return num1-num2;
    		case "*":
    			return num1*num2;
    		case "/":
    			return num1/num2;
    		}
    		return 0;
    }
  
    private float arrayValue(String array, int index) {
    		
 //   			System.out.println(array+":"+index);
    			for (int i = 0;i<this.arrays.size();i++) {
    				if (this.arrays.get(i).name.equals(array)) {
    					ArraySymbol x = this.arrays.get(i);
  //  					System.out.println(x);
    					return this.arrays.get(i).values[index];
    				}
    			}

    		return 0;
    }
    
    
    /**
     * Utility method, prints the symbols in the scalars list
     */
    public void printScalars() {
        for (ScalarSymbol ss: scalars) {
            System.out.println(ss);
        }
    }
    
    /**
     * Utility method, prints the symbols in the arrays list
     */
    public void printArrays() {
    		for (ArraySymbol as: arrays) {
    			System.out.println(as);
    		}
    }
    
    private void printStringArrays(String [] arr, int start, int end)
    {
//   		System.out.println("Evaluation string");
    		for (int i=start;i<=end;i++) {
  			System.out.print(arr[i]);
    		}
    		System.out.println("");
    }
}

