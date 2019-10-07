package math;

/**
 * This class encapsulates a BigInteger, i.e. a positive or negative integer with 
 * any number of digits, which overcomes the computer storage length limitation of 
 * an integer.
 * 
 */
public class BigInteger {

	/**
	 * True if this is a negative integer
	 */
	boolean negative;
	
	/**
	 * Number of digits in this integer
	 */
	int numDigits;
	
	/**
	 * Reference to the first node of this integer's linked list representation
	 * NOTE: The linked list stores the Least Significant Digit in the FIRST node.
	 * For instance, the integer 235 would be stored as:
	 *    5 --> 3  --> 2
	 */
	DigitNode front;
	
	/**
	 * Initializes this integer to a positive number with zero digits, in other
	 * words this is the 0 (zero) valued integer.
	 */
	public BigInteger() {
		negative = false;
		numDigits = 0;
		front = null;
	}
	
	/**
	 * Parses an input integer string into a corresponding BigInteger instance.
	 * A correctly formatted integer would have an optional sign as the first 
	 * character (no sign means positive), and at least one digit character
	 * (including zero). 
	 * Examples of correct format, with corresponding values
	 *      Format     Value
	 *       +0            0
	 *       -0            0
	 *       +123        123
	 *       1023       1023
	 *       0012         12  
	 *       0             0
	 *       -123       -123
	 *       -001         -1
	 *       +000          0
	 *       
	 * 
	 * @param integer Integer string that is to be parsed
	 * @return BigInteger instance that stores the input integer
	 * @throws IllegalArgumentException If input is incorrectly formatted
	 */
	public static BigInteger parse(String integer) 
	throws IllegalArgumentException {
		BigInteger number = new BigInteger(); 
		number.negative=false;
		
		if (integer.charAt(0) == '-') {
			number.negative = true;
			integer = integer.substring(1);
		}
		if (integer.charAt(0) == '+') {
			integer = integer.substring(1);
		}
		if (integer.charAt(0)==' ') {
			while (integer.charAt(0) == ' ') {
				integer=integer.substring(1);
			}
		}
		if (integer.charAt(integer.length()-1)==' ') {
			while (integer.charAt(integer.length()-1) == ' ') {
				integer = integer.substring(0, integer.length()-2);
			}
		}
		if (integer.charAt(0)=='0') {
			while (integer.charAt(0)=='0' && integer.length() > 1) {
				integer=integer.substring(1);	
			}
		}
		
		if (integer.length() == 1 && integer.charAt(0)=='0') {
			number.negative = false;
		}
		number.numDigits = 0;
		
		for (int i = 0; i<integer.length(); i++) {
			if (Character.isDigit(integer.charAt(i)) == false){
				throw new IllegalArgumentException("Incorrect Format");
			}
		}
		for (int i = 0; i<integer.length(); i++) {
			if (integer.charAt(i) == ' ') {
				throw new IllegalArgumentException("Incorrect Format");
			}
		}
		
		number.front = new DigitNode(Character.getNumericValue(integer.charAt(0)),null);
		
		if (integer.length()>1) {
			number.numDigits = 1;
			for (int i = 1; i<integer.length(); i++) {
				int indnumber = Character.getNumericValue(integer.charAt(i));
				
				//if the above thing is wrong then this entire thing doesnt work FIGURE THAT OUT
				number.front = new DigitNode(indnumber,number.front);
				number.numDigits++;
			}	
			
		}
		//System.out.println(number.numDigits);
		return number;
		//STILL NEED TO DO LETTERS AND SPACES
	}
	
	/**
	 * Adds an integer to this integer, and returns the result in a NEW BigInteger object. 
	 * DOES NOT MODIFY this integer.
	 * NOTE that either or both of the integers involved could be negative.
	 * (Which means this method can effectively subtract as well.)
	 * 
	 * @param other Other integer to be added to this integer
	 * @return Result integer
	 */
	public BigInteger add(BigInteger other) {
		BigInteger sum = new BigInteger(); 
		sum.front = null;
		DigitNode sumptr = null;
		int carry = 0;
		//traverse(other.front);
//subtraction when this is bigger than other
		if(this.negative != other.negative) {
			//traverse(other.front);
			//System.out.println("upthere: "+greaterThan(this,other));
			if (greaterThan(this,other) == 'a' || greaterThan(this,other) == 'e') {
				//System.out.print("other: ");
				//traverse(other.front);
				//System.out.println();
/*				if (greaterThan(this,other) == 'a') {
					DigitNode temp = other.front;
					while (temp.next != null) {
						temp = temp.next;
						temp.next = new DigitNode(0,null); //**NOT SURE IF THIS IS CORRECT**
					}
				}
*/					int borrow = 0;
					//traverse(other.front);
					//System.out.println();
					DigitNode thisptr = this.front;
					DigitNode otherptr = other.front;
					//traverse(other.front);
					int difference = 0;
					while (otherptr != null && thisptr != null) {
						int a = thisptr.digit;
						int b = otherptr.digit;
						//System.out.println("first");
						//System.out.println(otherptr.digit);
						if ((a-borrow)<b) {
							difference = a-borrow+10-b;
							borrow = 1;
						} else {
							difference = a-borrow-b;
						}
						//System.out.println("difference: "+difference);
						if (sum.front == null) {
							sum.front = new DigitNode(difference,null);
							sumptr = sum.front;
						} else {
							sumptr.next = new DigitNode(difference,null);
							sumptr = sumptr.next;
						}
						//System.out.println(sumptr.digit);
						//System.out.println("hello");
						thisptr = thisptr.next;
						otherptr = otherptr.next;
					}
					while (thisptr != null && otherptr == null) {
						difference = thisptr.digit-borrow;
						borrow = 0;
						sumptr.next = new DigitNode(difference,null);
						sumptr = sumptr.next;
						//System.out.println(difference);
						thisptr = thisptr.next;
					}
					
					if (this.negative == true) {
						sum.negative = true;
					}
					sum.front = deleteEndZeros(sum.front);
					//traverse(sum.front);
					sum.numDigits = count(sum.front);
					return sum;	
			} else if (greaterThan(this,other) == 'b' || greaterThan(this,other) == 'f') {
			
			//when other is greater than this
/*				if (greaterThan(this,other) == 'b') {
					DigitNode temp = this.front;
					while (temp.next != null) {
						temp = temp.next;
						temp.next = new DigitNode(0,null); //**NOT SURE IF THIS IS CORRECT**
					}
				}
*/	
				int borrow = 0;
				DigitNode thisptr = this.front;
				DigitNode otherptr = other.front;
				int difference = 0;
				while (otherptr != null && thisptr != null) {
					int a = otherptr.digit;
					int b = thisptr.digit;
					if ((a-borrow)<b) {
						difference = a-borrow+10-b;
						borrow = 1;
					} else {
						difference = a-borrow-b;
					}
					if (sum.front == null) {
						sum.front = new DigitNode(difference,null);
						sumptr = sum.front;
					} else {
						sumptr.next = new DigitNode(difference,null);
						sumptr = sumptr.next;
					}
					thisptr = thisptr.next;
					otherptr = otherptr.next;
				}
				while (otherptr != null && thisptr == null) {
					//System.out.println("heck");
					difference = otherptr.digit-borrow;
					borrow = 0;
					sumptr.next = new DigitNode(difference,null);
					sumptr=sumptr.next;
					otherptr = otherptr.next;
				}
				if (other.negative == true) {
					sum.negative = true;
				}
	/*			DigitNode temp = sum.front;
				DigitNode t = null;
				while (temp.next != null) {
					t = temp;
					temp = temp.next;
				}
				if (temp.digit == 0) {
					while (temp.digit == 0) {
						free(t.next);
						t.next = null;
					}
				}
				System.out.println("temp: ");
				
				traverse(temp);
				System.out.println("sum: ");
				traverse(sum.front);
*/
				sum.front = deleteEndZeros(sum.front);
				sum.numDigits = count(sum.front);
				return sum;	
			} else { 
				sum.front = new DigitNode(0,null);
				sum.numDigits = 0;
				return sum;
			} //DOn't think i need: (greaterThan(this,sum) == 'c') {
			
		} else {
//addition starts
			DigitNode thisptr = this.front;
			DigitNode otherptr = other.front;
			//System.out.println("xx");
			while (thisptr != null && otherptr != null) {
				int a = thisptr.digit;
				int b = otherptr.digit;
				int total = a+b+carry;
				carry = 0;
				if (total>= 10) {
					carry = 1;
					total = total%10;
				}
				if (sum.front == null) {
					sum.front = new DigitNode(total,null);
					sumptr = sum.front;
				} else {
					sumptr.next = new DigitNode(total,null);
					sumptr = sumptr.next;
				}
				thisptr = thisptr.next;
				otherptr = otherptr.next;
			}
			while (thisptr != null && otherptr == null) {
				int c = thisptr.digit + carry;
				carry = 0;
				if (c>= 10) {
					carry = 1;
					c = c%10;
				}
				sumptr.next = new DigitNode(c,null);
				sumptr = sumptr.next;
				thisptr = thisptr.next;
			}
			while (thisptr == null && otherptr != null) {
				int c = otherptr.digit + carry;
				//System.out.println("carry: " + carry);
				carry = 0;
				if(c>= 10) {
					carry = 1;
					c = c%10;
				}
				sumptr.next = new DigitNode(c,null);
				sumptr = sumptr.next;
				otherptr = otherptr.next;
			}
/*			deleteExtraZeros(sum);
			if (carry == 1) {
				sumptr.next = new DigitNode(1,null);
				sumptr = sumptr.next;
			}
*/
			if (this.negative == true && other.negative == true) {
				sum.negative = true;
			}
		}
		sum.numDigits = count(sum.front);
		if (sum.numDigits == 1 && sum.front.digit == 0) {
			sum.numDigits = 0;
		}
		//System.out.println(sum.numDigits);
		return sum;
	}
		
	//NEED TO DO NEGATIVES
	
	
	/**
	 * Returns the BigInteger obtained by multiplying the given BigInteger
	 * with this BigInteger - DOES NOT MODIFY this BigInteger
	 * 
	 * @param other BigInteger to be multiplied
	 * @return A new BigInteger which is the product of this BigInteger and other.
	 */
	public BigInteger multiply(BigInteger other) {
		
		//System.out.println("m: "+thisptr.digit);
		DigitNode otherptr = other.front;
		//System.out.println("m: "+otherptr.digit);
		BigInteger total = new BigInteger();
		total.front = new DigitNode(0,null);
		
		
			int i = 0;
			while (otherptr != null) {
				//traverse(otherptr);
				DigitNode thisptr = this.front;
				BigInteger sum1 = new BigInteger();
				sum1.front = null;
				DigitNode sumptr = null;
				i++;
				int carry = 0;
				while (thisptr != null) {
					//traverse(thisptr);
					int a = thisptr.digit * otherptr.digit + carry;
					carry = 0;
					if (a>=10) {
						int temp = a%10;
						carry = (a-temp)/10;
						a = temp;
					}
					//System.out.println(a);
					//System.out.println("Goes through once");
					if (sum1.front == null) {
						sum1.front = new DigitNode(a,null);
						sumptr = sum1.front;
					} else {
						sumptr.next = new DigitNode(a,null);
						sumptr = sumptr.next;
					}
					//System.out.println("xx: ");
					//traverse(sum1.front);
					thisptr = thisptr.next;
					}
				//System.out.print("cy: ");
				//traverse(sum1.front);
				//System.out.println();
				if (carry != 0) {
					sumptr.next = new DigitNode(carry,null);
					sumptr = sumptr.next;
				}
				//System.out.print("Problem here?: ");
				//traverse(sum1.front);
				//System.out.println("i: "+i);
				for (int j = i; j>0; j--) {
					if (j == 1) {
						continue;
					} else {
						sum1.front = new DigitNode(0,sum1.front);
					}
				}
				//System.out.println("Not there, so here?: ");
				//traverse(sum1.front);
				//System.out.println("Here it is");
				//traverse(sum1.front);
				//System.out.println();
				//traverse(sum1.front);
				//traverse(total.front);
				//System.out.println("Total:");
				total = sum1.add(total);
				//System.out.println("x");
				//traverse(total.front);
				otherptr = otherptr.next;
				//traverse(otherptr);
				//traverse(thisptr);
			}
		
		
		if ((this.negative == true && other.negative == false) || (this.negative == false && other.negative == true)) {
			total.negative = true;
		}
		if (this.negative == true && other.negative == true) {
			total.negative = false;
		}
		total.numDigits = count(total.front);
		if (total.numDigits == 1 && total.front.digit == 0) {
			total.numDigits = 0;
		}
		return total;
	}
	
	/*
	 * key:
	 * a : a>b, a is longer than b
	 * b : b>a, b is longer than a
	 * c : numbers are equal
	 * e : same length, a>b
	 * f : same length, b>a
	 */
/*	private char greaterThan(BigInteger a, BigInteger b) {
		DigitNode ptra = a.front;
		DigitNode ptrb = b.front;
		if (a.numDigits > b.numDigits) {
			return 'a'; //(*a means that a is greater than b)
		}
		if (a.numDigits < b.numDigits) {
			return 'b'; //(*b means that b is greater than a)
		}
		if (a.numDigits == b.numDigits) {
			DigitNode aPTR = ptra;
			DigitNode aPREV = null;
			DigitNode aNEXT = null;
			DigitNode bPTR = ptrb;
			DigitNode bPREV = null;
			DigitNode bNEXT = null;
			for (int i = 1; i<=a.numDigits; i++) {
				//System.out.println("pointer.next: "+aPTR.next);
				if (aPTR.next == null) {
					aNEXT = null;
				} else {
					aNEXT = aPTR.next;
				}
				aPTR.next = aPREV;
				aPREV = aPTR;
				aPTR = aNEXT;
				
				bNEXT = bPTR.next;
				bPTR.next = bPREV;
				bPREV = bPTR;
				bPTR = bNEXT;
			}
			ptra = aPREV;
			ptrb = bPREV;
			for (int j = 1; j<=a.numDigits; j++) {
				if (ptra.digit > ptrb.digit) {
					return 'e';
				} else if (ptrb.digit>ptra.digit) {
					return 'f';
				} else {
					ptra = ptra.next;
					ptrb = ptrb.next;
				}
			}
		}
		return 'c';
	}
*/
/*	private static char greaterThan(BigInteger a, BigInteger b) {
		BigInteger tempa = a;
		BigInteger tempb = b;
		char answer = 0;
		if (tempa.numDigits > tempb.numDigits) {
			answer = 'a';
		} else if (tempb.numDigits > tempa.numDigits) {
			answer = 'b';
		} else {
			System.out.println("x");
			DigitNode reversea = reverse(tempa.front);
			traverse(reversea);
			DigitNode reverseb = reverse(tempb.front);
			traverse(reverseb);
			while (reversea != null) {
				if (reversea.digit > reverseb.digit) {
					answer = 'e';
					break;
				} else if (reverseb.digit > reversea.digit) {
					answer = 'f';
					break;
				} else {
					reversea = reversea.next;
					reverseb = reverseb.next;
					answer = 'c';
				}
			}
		}
		return answer;
	}
	private static DigitNode reverse(DigitNode f) {
		DigitNode prev = null;
		DigitNode current = f;
		DigitNode next;
		while (current != null) {
			next = current.next;
			current.next = prev;
			prev = current = next;
			current = next;
		}
		f = prev;
		return f;
	}
*/
	private static char greaterThan(BigInteger a,BigInteger b) {
		char answer = 'w';
		//System.out.println("x");
		if (a.numDigits>b.numDigits) {
			answer = 'a';
		} else if (b.numDigits > a.numDigits) {
			answer = 'b';
		} else {
			//System.out.println("xx");
			DigitNode ptra = a.front;
			DigitNode ptrb = b.front;
			//traverse(ptra);
			//traverse(ptrb);
			//System.out.println(ptra.digit);
			//System.out.println(ptrb.digit);
			//ptra = ptra.next;
			//System.out.println(ptra.digit);
			int i = 1;
			while (ptra != null) {
				if (i ==1 ) {
					if (ptra.digit > ptrb.digit) {
						answer = 'e';
						//System.out.println("correct");
					} else if (ptrb.digit > ptra.digit) {
						answer = 'f';
					} else {
						answer = 'c';
					}
				} else {
					if (ptra.digit > ptrb.digit) {
						answer = 'e';
					} else if (ptrb.digit > ptra.digit) {
						answer = 'f';
					}
				}
				//System.out.println(ptra.digit);
				//System.out.println(ptrb.digit);
				//System.out.println("next");
				
				ptra = ptra.next;
				ptrb = ptrb.next;
				i++;
			}
		}
		//System.out.println("answer: "+answer);
		return answer;
	}
/*	private char greaterThan (BigInteger a, BigInteger b) {
		char answer = 'w';
		if (a.numDigits > b.numDigits) {
			return 'a';
		}
		if (b.numDigits > a.numDigits) {
			return 'b';
		}
		if (a.numDigits == b.numDigits) {
			DigitNode ptra = a.front;
			DigitNode ptrb = b.front;
			while (ptra != null) {
				if (ptra.digit == ptrb.digit) {
					answer = 'c';
				}
				if (ptra.digit > ptrb.digit) {
					answer = 'e';
				}
				if (ptrb.digit > ptra.digit) {
					answer = 'f';
				}
				ptra = ptra.next;
				ptrb = ptrb.next;
			}
		}
		return answer;
	}
*/	/* (non-Javadoc)
	 * @see java.lang.Object#toString()
	 */
	public String toString() {
		if (front == null) {
			return "0";
		}
		
		String retval = front.digit + "";
		for (DigitNode curr = front.next; curr != null; curr = curr.next) {
				retval = curr.digit + retval;
		}
		
		if (negative) {
			retval = '-' + retval;
		}
		
		return retval;
	}

	/*private static void traverse(DigitNode front) {
		DigitNode ptr = front;
		while (ptr != null) {
			System.out.print(ptr.digit + " -> ");
			ptr = ptr.next;
		}
	}*/
	private static DigitNode deleteEndZeros(DigitNode front) {
		int temp = count(front);
		DigitNode nend = front;
		DigitNode n1 = null;
		//System.out.println(temp);
		if (temp > 1) {
			n1 = null;
			DigitNode n2 = front;
			while (n2.next != null) {
				n1 = n2;
				n2 = n2.next;
			}
			if (n1.next.digit == 0) {
				nend = n1.next;
				nend = null;
				deleteEndZeros(nend);
			}
		} else {
			nend = front;
		}
		return nend;
	}
	private static int count(DigitNode front) {
		int i = 0;
		DigitNode ptr = front;
		while (ptr != null) {
			i++;
			ptr = ptr.next;
		}
		//System.out.println("count: "+i);
		return i;
	}
	
}
