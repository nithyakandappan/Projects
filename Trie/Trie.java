package trie;

import java.util.ArrayList;

/**
 * This class implements a Trie. 
 * 
 * @author Sesh Venugopal
 *
 */
public class Trie {
	
	// prevent instantiation
	private Trie() { }
	
	/**
	 * Builds a trie by inserting all words in the input array, one at a time,
	 * in sequence FROM FIRST TO LAST. (The sequence is IMPORTANT!)
	 * The words in the input array are all lower case.
	 * 
	 * @param allWords Input array of words (lowercase) to be inserted.
	 * @return Root of trie with all words inserted from the input array
	 */
	public static TrieNode buildTrie(String[] allWords) {
		/** COMPLETE THIS METHOD **/
		int length = allWords[0].length() - 1;
		Indexes firstIndex = new Indexes(0,(short)0,(short)length);
		
		TrieNode firstWord = new TrieNode(firstIndex,null,null);
		TrieNode root = new TrieNode(null,firstWord,null);
		for (int i = 1;i<allWords.length;i++) {
			String word = allWords[i];
			addToRoot(root,word,allWords,i,0);
		}
		
		return root;
	}
	private static TrieNode addToRoot(TrieNode root, String word, String[] all,int i, int comparison) {
		TrieNode prev = root;
		TrieNode ptr = root.firstChild;
//		System.out.println(word);
		while (ptr.sibling !=null && compare(all[ptr.substr.wordIndex],word,comparison) == -1) {
			ptr = ptr.sibling;
//			System.out.println("while: "+i);
		}
		if (compare(all[ptr.substr.wordIndex],word,comparison) == -1) {
			int length = word.length()-1;
//			System.out.println(word+":"+length);
//			System.out.println("comparison: "+word+":"+compare(all[ptr.substr.wordIndex],word,comparison));
			Indexes newIndex = new Indexes(i,(short)comparison,(short) length);
			ptr.sibling = new TrieNode(newIndex,null,null);
//			System.out.println("if1: "+i);
		} else if (compare(all[ptr.substr.wordIndex],word,comparison) != -1) {
			if (compare(all[ptr.substr.wordIndex],word,comparison)<ptr.substr.endIndex && ptr.firstChild != null) {
//				System.out.println("COMPARISON: "+compare(all[ptr.substr.wordIndex],word,comparison)+"and ENDINDEX:"+ptr.substr.endIndex);
//				System.out.println("HERE I AM");
				TrieNode tempsib = ptr.sibling;
				TrieNode temp = ptr;
				temp.sibling = null;
				int x = compare(all[ptr.substr.wordIndex],word,comparison);
				int y = x+1;
				Indexes newIndex = new Indexes(ptr.substr.wordIndex,(short) comparison,(short) x);
				temp.substr.startIndex = (short) y;
				prev.firstChild = new TrieNode(newIndex,temp,tempsib);
//				ptr = prev.firstChild;
//				ptr.sibling = null;
				addToRoot(prev,word,all,i,comparison);
			} else {
				if (ptr.substr.endIndex != all[ptr.substr.wordIndex].length()-1) {
					String x = all[ptr.substr.wordIndex].substring(ptr.substr.startIndex, ptr.substr.endIndex+1);
					if (word.contains(x)) {
						addToRoot(ptr,word,all,i,ptr.substr.endIndex+1);
					}
				} else {
					if (ptr.firstChild == null) {
//						System.out.println("if2: "+word);
						int intersect = compare(all[ptr.substr.wordIndex],word,comparison);
						int length = word.length()-1;
						int originalEndIndex = ptr.substr.endIndex;
						ptr.substr.endIndex = (short) intersect;
						int whereToStart = intersect+1;
						Indexes IndexOfChild = new Indexes(ptr.substr.wordIndex,(short) whereToStart, (short) originalEndIndex);
						ptr.firstChild  = new TrieNode(IndexOfChild,null,null);
						addToRoot(ptr,word,all,i,whereToStart);
					} else {
						int intersect = compare(all[ptr.substr.wordIndex],word,comparison);
						int whereToStart = intersect+1;
						addToRoot(ptr,word,all,i,whereToStart);
					}
				}
			
			}	
		}
//		} else if (ptr.sibling != null && compare(all[ptr.substr.wordIndex],word,comparison) != -1) {
//			//this part of the else if statement needs to be fixes
//			if (ptr.substr.endIndex != all[ptr.substr.wordIndex].length()-1) {
//				String x = all[ptr.substr.wordIndex].substring(ptr.substr.startIndex, ptr.substr.endIndex+1);
//				if (word.contains(x)) {
//					addToRoot(ptr,word,all,i,ptr.substr.endIndex+1);
//				}
//			} else {
//				if (ptr.firstChild == null) {
//					System.out.println("if2: "+word);
//					int intersect = compare(all[ptr.substr.wordIndex],word,comparison);
//					int length = word.length()-1;
//					int originalEndIndex = ptr.substr.endIndex;
//					ptr.substr.endIndex = (short) intersect;
//					int whereToStart = intersect+1;
//					Indexes IndexOfChild = new Indexes(ptr.substr.wordIndex,(short) whereToStart, (short) originalEndIndex);
//					ptr.firstChild  = new TrieNode(IndexOfChild,null,null);
//					addToRoot(ptr,word,all,i,whereToStart);
//				} else {
//					int intersect = compare(all[ptr.substr.wordIndex],word,comparison);
//					int whereToStart = intersect+1;
//					addToRoot(ptr,word,all,i,whereToStart);
//				}
//			}
//		}
		return root;
	}
//	public static TrieNode buildTrie2(String[] allWords) {
//		int length = allWords[0].length() - 1;
//		Indexes firstIndex = new Indexes(0,(short)0,(short)length);
//		TrieNode firstWord = new TrieNode(firstIndex,null,null);
//		TrieNode root = new TrieNode(null,firstWord,null);
//		
//		if (allWords.length>1) {
//			for (int i = 1;i<allWords.length;i++) {
//				String word = allWords[i];
//				TrieNode ptr = root;
//				while (ptr!=null) {
//					ptr = ptr.firstChild;
//					if (ptr.)
//				}
//			}
//		}
//	
//	}
	
//	public static TrieNode buildingTrie(TrieNode startNode, String[] allWords, int index) {
//		for (int i = 1;i<allWords.length;i++) {
//			System.out.println("round: "+i);
//			System.out.println(allWords[i-1]);
//			System.out.println(allWords[i]);
//			if (compare(allWords[i],allWords[i-1]) == -1) {
//				System.out.println("if");
//				int wordLength = allWords[i].length()-1;
//				System.out.println("wordLength: "+wordLength);
//				Indexes newIndex = new Indexes(i,(short)0,(short)wordLength);
//				System.out.println("newIndex: "+newIndex);
//				System.out.println("firstChild: "+startNode.firstChild);
//				startNode.firstChild.sibling = new TrieNode(newIndex,null,null); 
//				System.out.println("Node: "+startNode.sibling);
////			} else {
////				int wordLength = allWords[i].length();
////				Inde
//			}
//		}
//		return startNode;
//	}
	
	private static int compare(String a, String b,int x) {
		if (a.length()>b.length()){
			if (a.charAt(x) != b.charAt(x)) {
//				System.out.println("a: "+a.charAt(x));
//				System.out.println("b: "+b.charAt(x));
				return -1;
			} else {
				for (int i = x;i<b.length();i++) {
					if (a.charAt(i) != b.charAt(i)) {
						return i-1;
					}
				}
			}
			
		} else if (a.length()<b.length()){
			if (b.contains(a) == true) {
				return -2;
			}
			if (a.charAt(x) != b.charAt(x)) {
				return -1;
			} else {
				for (int i = x;i<a.length();i++) {
					if (a.charAt(i) != b.charAt(i)) {
						return i-1;
					}
				}
			}
		} else {
			if (a.charAt(x) != b.charAt(x)) {
				return -1;
			} else {
				for (int i = x;i<b.length();i++) {
					if (a.charAt(i) != b.charAt(i)) {
						return i-1;
					}
				}
			}
		}
		return -1;
	}
	/**
	 * Given a trie, returns the "completion list" for a prefix, i.e. all the leaf nodes in the 
	 * trie whose words start with this prefix. 
	 * For instance, if the trie had the words "bear", "bull", "stock", and "bell",
	 * the completion list for prefix "b" would be the leaf nodes that hold "bear", "bull", and "bell"; 
	 * for prefix "be", the completion would be the leaf nodes that hold "bear" and "bell", 
	 * and for prefix "bell", completion would be the leaf node that holds "bell". 
	 * (The last example shows that an input prefix can be an entire word.) 
	 * The order of returned leaf nodes DOES NOT MATTER. So, for prefix "be",
	 * the returned list of leaf nodes can be either hold [bear,bell] or [bell,bear].
	 *
	 * @param root Root of Trie that stores all words to search on for completion lists
	 * @param allWords Array of words that have been inserted into the trie
	 * @param prefix Prefix to be completed with words in trie
	 * @return List of all leaf nodes in trie that hold words that start with the prefix, 
	 * 			order of leaf nodes does not matter.
	 *         If there is no word in the tree that has this prefix, null is returned.
	 */
	public static ArrayList<TrieNode> completionList(TrieNode root,
										String[] allWords, String prefix) {
		/** COMPLETE THIS METHOD **/
		
		// FOLLOWING LINE IS A PLACEHOLDER TO ENSURE COMPILATION
		// MODIFY IT AS NEEDED FOR YOUR IMPLEMENTATION
		if (prefix.matches("")) {
			return null;
		}
		TrieNode ptr = placementx(root,prefix,allWords,0,0);
		if (ptr == null) {
			return null;
		}
		return createList(ptr,0);
	}
	private static TrieNode placementx(TrieNode root,String prefix,String[] allWords,int StartIndex,int i) {
		TrieNode ptr = root.firstChild;
		while (ptr!=null) {
			
			String x = convertToString(allWords,ptr);
//			System.out.println(x);
				if (prefix.startsWith(x)) {
					if (prefix.matches(x)) {
						return ptr;
					} else {
						if (placementx(ptr,prefix,allWords,StartIndex,i+1) == null) {
//							System.out.println("HERE");
							TrieNode nextptr = ptr.firstChild;
							while (nextptr != null) {
								String z = convertToString(allWords,nextptr);
								if (z.startsWith(prefix)) {
									return nextptr;
								}
								nextptr = nextptr.sibling;
							}
							return null;
						} else {
							return placementx(ptr,prefix,allWords,StartIndex,i+1);
						}
					}
				} else if (x.startsWith(prefix)){
					return ptr;
				} else {
					ptr = ptr.sibling;
				}
			}
//		System.out.println("DONE");
		return null;
		}
	private static ArrayList<TrieNode> createList(TrieNode root,int i){
		TrieNode ptr = root;
		ArrayList<TrieNode> list = new ArrayList<TrieNode>();
		if (i==0) {
			if (ptr.firstChild == null) {
				list.add(ptr);
			} else {
				list.addAll(createList(ptr.firstChild,i+1));
			}
		} else {
			while (ptr!=null) {
				if (ptr.firstChild == null) {
					list.add(ptr);
				} else {
					list.addAll(createList(ptr.firstChild,i+1));
				}
				ptr = ptr.sibling;
			}
		}
		return list;
	}
	
	public static void print(TrieNode root, String[] allWords) {
		System.out.println("\nTRIE\n");
		print(root, 1, allWords);
	}
	
	private static void print(TrieNode root, int indent, String[] words) {
		if (root == null) {
			return;
		}
		for (int i=0; i < indent-1; i++) {
			System.out.print("    ");
		}
		
		if (root.substr != null) {
			String pre = words[root.substr.wordIndex]
							.substring(0, root.substr.endIndex+1);
			System.out.println("      " + pre);
		}
		
		for (int i=0; i < indent-1; i++) {
			System.out.print("    ");
		}
		System.out.print(" ---");
		if (root.substr == null) {
			System.out.println("root");
		} else {
			System.out.println(root.substr);
		}
		
		for (TrieNode ptr=root.firstChild; ptr != null; ptr=ptr.sibling) {
			for (int i=0; i < indent-1; i++) {
				System.out.print("    ");
			}
			System.out.println("     |");
			print(ptr, indent+1, words);
		}
	}
	private static String convertToString(String[] all,TrieNode ptr) {
		String x = "";
		if (ptr.substr.endIndex == all[ptr.substr.wordIndex].length()-1) {
			x = all[ptr.substr.wordIndex].substring(0);
		} else {
			x = all[ptr.substr.wordIndex].substring(0,ptr.substr.endIndex+1);
		}
		return x;
	}

 }
