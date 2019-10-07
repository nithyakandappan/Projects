package lse;

import java.io.*;
import java.util.*;

/**
 * This class builds an index of keywords. Each keyword maps to a set of pages in
 * which it occurs, with frequency of occurrence in each page.
 *
 */
public class LittleSearchEngine {
	
	/**
	 * This is a hash table of all keywords. The key is the actual keyword, and the associated value is
	 * an array list of all occurrences of the keyword in documents. The array list is maintained in 
	 * DESCENDING order of frequencies.
	 */
	HashMap<String,ArrayList<Occurrence>> keywordsIndex;
	
	/**
	 * The hash set of all noise words.
	 */
	//BELOW SHOULD NOT BE STATIC
	HashSet<String> noiseWords;
	
	/**
	 * Creates the keyWordsIndex and noiseWords hash tables.
	 */
	public LittleSearchEngine() {
		keywordsIndex = new HashMap<String,ArrayList<Occurrence>>(1000,2.0f);
		noiseWords = new HashSet<String>(100,2.0f);
	}
	
	/**
	 * Scans a document, and loads all keywords found into a hash table of keyword occurrences
	 * in the document. Uses the getKeyWord method to separate keywords from other words.
	 * 
	 * @param docFile Name of the document file to be scanned and loaded
	 * @return Hash table of keywords in the given document, each associated with an Occurrence object
	 * @throws FileNotFoundException If the document file is not found on disk
	 */
	public HashMap<String,Occurrence> loadKeywordsFromDocument(String docFile) 
	throws FileNotFoundException {
		/** COMPLETE THIS METHOD **/
/**/		HashMap<String,Occurrence> docKeywords = new HashMap<String,Occurrence>();
		Scanner reader = new Scanner(new File(docFile));
		int j = 0;
//		ArrayList<String> tokens = new ArrayList<String>();
		while (reader.hasNextLine()) {
			String line = reader.nextLine();
//			System.out.println("line: "+docFile+" : "+line);
			StringTokenizer st = new StringTokenizer(line);
			while (st.hasMoreTokens()) {
				j++;
//				System.out.println(docFile + " : "+j);
				String token = getKeyword(st.nextToken());
				if (token != null){
					if (docKeywords.get(token) == null) {
						Occurrence tokenOccurrence = new Occurrence(docFile,1);
		/**/				docKeywords.put(token, tokenOccurrence);
					} else {
						int freq = docKeywords.get(token).frequency;
						freq++;
						docKeywords.get(token).frequency = freq;
//						System.out.println(docFile+" : "+token+" : "+freq);
					}
				}
//				while ()
//				System.out.println("frequency: "+docFile+" : "+);
//				System.out.println("token: "+docFile+" : "+token);
			}
		}
//		System.out.println(j);
//		System.out.println(docKeywords.size());
		// following line is a placeholder to make the program compile
		// you should modify it as needed when you write your code
		return docKeywords;
	}
	
	/**
	 * Merges the keywords for a single document into the master keywordsIndex
	 * hash table. For each keyword, its Occurrence in the current document
	 * must be inserted in the correct place (according to descending order of
	 * frequency) in the same keyword's Occurrence list in the master hash table. 
	 * This is done by calling the insertLastOccurrence method.
	 * 
	 * @param kws Keywords hash table for a document
	 */
	public void mergeKeywords(HashMap<String,Occurrence> kws) {
//		System.out.println("hello");
//		if (kws.isEmpty()) {
//			System.out.println("0");
//			return;
//		}
		for (String key:kws.keySet()) {
//			System.out.println(key);
			if (keywordsIndex.containsKey(key)) {
				keywordsIndex.get(key).add(kws.get(key));
				insertLastOccurrence(keywordsIndex.get(key));
			} else {
				ArrayList<Occurrence> temp = new ArrayList<Occurrence>();
				temp.add(kws.get(key));
				insertLastOccurrence(temp);
				keywordsIndex.put(key,temp);
			}
		}
		/** COMPLETE THIS METHOD **/
	}
	
	/**
	 * Given a word, returns it as a keyword if it passes the keyword test,
	 * otherwise returns null. A keyword is any word that, after being stripped of any
	 * trailing punctuation, consists only of alphabetic letters, and is not
	 * a noise word. All words are treated in a case-INsensitive manner.
	 * 
	 * Punctuation characters are the following: '.', ',', '?', ':', ';' and '!'
	 * 
	 * @param word Candidate word
	 * @return Keyword (word without trailing punctuation, LOWER CASE)
	 */
	//BELOW SHOULD NOT BE STATIC //NOT DONE(NOISEWORDS)
	public String getKeyword(String word) {
		/** COMPLETE THIS METHOD **/ 
		if (word == null) {
			return null;
		}
		int i = word.length()-1;
		while (i>-1 && Character.isLetter(word.charAt(i)) != true) {
			i--;
		}
		String temp = null;
//		System.out.println("hi");
		if (i==-1) {
			return null;
		} else {
			if (i == word.length()-1) {
				temp = word;
			} else {
				temp = word.substring(0,i+1);
			}
		}
		for (int j = 0;j<temp.length();j++) {
			if (Character.isLetter(temp.charAt(j)) != true) {
				return null;
			}
		}
		temp = temp.toLowerCase();
		if (noiseWords.contains(temp)) {
			return null;
		} else {
			return temp;
		}
		
			// following line is a placeholder to make the program compile
		// you should modify it as needed when you write your code
	}
	
	/**
	 * Inserts the last occurrence in the parameter list in the correct position in the
	 * list, based on ordering occurrences on descending frequencies. The elements
	 * 0..n-2 in the list are already in the correct order. Insertion is done by
	 * first finding the correct spot using binary search, then inserting at that spot.
	 * 
	 * @param occs List of Occurrences
	 * @return Sequence of mid point indexes in the input list checked by the binary search process,
	 *         null if the size of the input list is 1. This returned array list is only used to test
	 *         your code - it is not used elsewhere in the program.
	 */
	//IT"S NOT DONE //SHOULD NOT BE STATIC
	public ArrayList<Integer> insertLastOccurrence(ArrayList<Occurrence> occs) {
		/** COMPLETE THIS METHOD **/
		ArrayList<Integer> number = new ArrayList<>();
		int size = occs.size();
		if (occs.size()==1) {
			return null;
		}
		Occurrence temp = occs.get(occs.size()-1);
		occs.remove(occs.size()-1);
		int targfreq = temp.frequency;
		int hi = 0;
		int lo = occs.size()-1;
		while (hi<lo) {
			int middle = (lo+hi)/2;
			number.add(middle);
	//		System.out.println(middle);
			Occurrence mid = occs.get(middle);
			if (mid.frequency == targfreq) {
				occs.add(middle,temp);
			}
			if (mid.frequency > targfreq) {
				hi = middle+1;
			} else {
				lo = middle;
			}
		}
		if (occs.size()<size) {
			occs.add(lo,temp);
		}
//		Below exists just to check stuff
//		int x = occs.size();
//		for (int i = 0;i<x;i++) {
//			System.out.println(occs.get(i));
//		}
		// following line is a placeholder to make the program compile
		// you should modify it as needed when you write your code
		return number;
	}
	
	/**
	 * This method indexes all keywords found in all the input documents. When this
	 * method is done, the keywordsIndex hash table will be filled with all keywords,
	 * each of which is associated with an array list of Occurrence objects, arranged
	 * in decreasing frequencies of occurrence.
	 * 
	 * @param docsFile Name of file that has a list of all the document file names, one name per line
	 * @param noiseWordsFile Name of file that has a list of noise words, one noise word per line
	 * @throws FileNotFoundException If there is a problem locating any of the input files on disk
	 */
	public void makeIndex(String docsFile, String noiseWordsFile) 
	throws FileNotFoundException {
		// load noise words to hash table
		Scanner sc = new Scanner(new File(noiseWordsFile));
		while (sc.hasNext()) {
			String word = sc.next();
			noiseWords.add(word);
		}
		
		// index all keywords
		sc = new Scanner(new File(docsFile));
		while (sc.hasNext()) {
			String docFile = sc.next();
			HashMap<String,Occurrence> kws = loadKeywordsFromDocument(docFile);
			mergeKeywords(kws);
		}
		sc.close();
	}
	
	/**
	 * Search result for "kw1 or kw2". A document is in the result set if kw1 or kw2 occurs in that
	 * document. Result set is arranged in descending order of document frequencies. (Note that a
	 * matching document will only appear once in the result.) Ties in frequency values are broken
	 * in favor of the first keyword. (That is, if kw1 is in doc1 with frequency f1, and kw2 is in doc2
	 * also with the same frequency f1, then doc1 will take precedence over doc2 in the result. 
	 * The result set is limited to 5 entries. If there are no matches at all, result is null.
	 * 
	 * @param kw1 First keyword
	 * @param kw1 Second keyword
	 * @return List of documents in which either kw1 or kw2 occurs, arranged in descending order of
	 *         frequencies. The result size is limited to 5 documents. If there are no matches, returns null.
	 */
	public ArrayList<String> top5search(String kw1, String kw2) {
		/** COMPLETE THIS METHOD **/
		ArrayList<Occurrence> kw1list = null;
		ArrayList<Occurrence> kw2list = null;
		if (keywordsIndex.containsKey(kw1)) {
			kw1list = keywordsIndex.get(kw1);
		}
		if (keywordsIndex.containsKey(kw2)) {
			kw2list = keywordsIndex.get(kw2);
		}
		ArrayList<Occurrence> end = null;
		if (kw1list == null || kw2list == null) {
			if (kw1list == null) {
				end = kw2list;
			} else if (kw2list == null){
				end = kw1list;
			} else {
				end = null;
			}
		} else {
			while (kw1list.size() != 0) {
				kw2list.add(kw1list.remove(0));
				insertLastOccurrence(kw2list);
			}
//			if (kw1list.size()<kw2list.size()) {
//				while (kw1list.size() != 0) {
//					kw2list.add(kw1list.remove(0));
//					insertLastOccurrence(kw2list);
//				}
//				end = kw2list;
//			} else {
//				while (kw2list.size() != 0) {
////					System.out.println("size: "+kw1list.size());
//					kw1list.add(kw2list.remove(0));
//					insertLastOccurrence(kw1list);
//				}
//				end = kw1list;
//			}
			end = kw2list;
		}
		ArrayList<String> docs = new ArrayList<String>();
		if (end != null) {
			int j = 1;
			while (j!=6 && end.size()!=0) {
				Occurrence temp = end.remove(0);
				if (docs.indexOf(temp.document) == -1) {
//					System.out.println("freq: "+temp.frequency);
					docs.add(temp.document);
				}
				j++;
//				docs.add(temp.document);
			}
		} else {
			docs = null;
		}
		
		
		// following line is a placeholder to make the program compile
		// you should modify it as needed when you write your code
		return docs;
	
	}
//}
//	private static int binarySearch(int[] arr) {
//		return null;
//	}
//	public static void main(String[] args) {
////		System.out.println("hello");
//		String txt = getKeyword("Through");
//		System.out.println(txt);
////		ArrayList<Occurrence> hello = new ArrayList<Occurrence>(7);
////		hello.add(new Occurrence("hello",12));
////		hello.add(new Occurrence("hello",8));
////		hello.add(new Occurrence("hello",7));
////		hello.add(new Occurrence("hello",5));
////		hello.add(new Occurrence("hello",3));
////		hello.add(new Occurrence("hello",2));
////		hello.add(new Occurrence("hello",13));
////		insertLastOccurrence(hello);
//	}
//	public static void main(String [] args)
//	{
//		LittleSearchEngine lse = new LittleSearchEngine();
//		try {
//			lse.makeIndex("docs.txt", "noisewords.txt");
//		}
//		catch (FileNotFoundException e) {
//			System.out.println("Cannot find file");
//		};
////		ArrayList<String> answer = 
////		if (lse.top5search("terytr", "hfddhju") == null) {
////			System.out.println("0");
////		}
////		if (answer.isEmpty()) {
////			System.out.println("0");
////		}
////		while (answer.size() != 0) {
//////			System.out.println("hi");
////			System.out.println(answer.remove(0));
////		}
////		hashPrint(lse.keywordsIndex);
////		System.out.println(lse.noiseWords);
////		
////		System.out.println("This is the main driver");
//	}
}
