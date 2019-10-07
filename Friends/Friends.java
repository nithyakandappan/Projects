package friends;

import structures.Queue;
import structures.Stack;

import java.io.File;
import java.io.FileNotFoundException;
import java.util.*;

public class Friends {

	/**
	 * Finds the shortest chain of people from p1 to p2.
	 * Chain is returned as a sequence of names starting with p1,
	 * and ending with p2. Each pair (n1,n2) of consecutive names in
	 * the returned chain is an edge in the graph.
	 * 
	 * @param g Graph for which shortest chain is to be found.
	 * @param p1 Person with whom the chain originates
	 * @param p2 Person at whom the chain terminates
	 * @return The shortest chain from p1 to p2. Null if there is no
	 *         path from p1 to p2
	 */
	
	//CHECK ORDER
	public static ArrayList<String> shortestChain(Graph g, String p1, String p2) {
		
		int i = 0;
		Person firstPerson = g.members[0];
		while (firstPerson.name.equals(p1) != true) {
			i++;
			firstPerson = g.members[i];
		}
		int k = 0;
		Person secondPerson = g.members[0];
		while (secondPerson.name.equals(p2)!=true) {
			k++;
			secondPerson = g.members[k];
		}
//		System.out.println("l");
		ArrayList<String> path = new ArrayList<String>();
		ArrayList<Person> answer = Chains (g,firstPerson,secondPerson);
		if (answer == null) {
			return null;
		}
		int size = answer.size();
//		System.out.println("size: "+size);
		for (int j = size-1;j>=0;j--) {
			String select = answer.get(j).name;
			path.add(select);
			System.out.println(select);
		}
		
		return path;
	}
	
	/*
	 * The method below is a private method that I created that is recursive.  
	 * It will create a list of all of the 
	 */
	private static ArrayList<Person> Chains(Graph g, Person p1, Person p2){
//		System.out.println("0");
		Person startPerson = p1;
		Queue<Person> list = new Queue<Person>();
		HashMap<Person,Boolean> alreadyVisited = new HashMap<Person,Boolean>();
		HashMap<Person,Person> previous = new HashMap<Person,Person>();
		ArrayList<Person> path = new ArrayList<Person>();
		alreadyVisited.put(p1, true);
		list.enqueue(p1);
		Person temp = null;
//		System.out.println("hello");
//		Friend example = g.members[1].first;
//		while (example !=null) {
//			System.out.println(g.members[example.fnum].name);
//			example = example.next;
//		}
//		System.out.println("end");
		while (list.isEmpty() != true) {
			
			temp = list.dequeue();
//			System.out.println(temp.name);
			if (temp.name.equals(p2.name)) {
				break;
			} else {
				Friend firstFriend = temp.first;
				while (firstFriend != null) {
//					System.out.println("hells bells");
					if (alreadyVisited.containsKey(g.members[firstFriend.fnum]) == false) {
						list.enqueue(g.members[firstFriend.fnum]);
						alreadyVisited.put(g.members[firstFriend.fnum], true);
						previous.put(g.members[firstFriend.fnum], temp);
					}
					firstFriend = firstFriend.next;
				}
			}
		}
		
		if (temp.equals(p2) == false) {
			
			return null;
		} else {
//			System.out.println("else");
			for (Person p = p2;p!=null;p = previous.get(p)) {
				path.add(p);
			}
		}
//		System.out.println("end");
		if (path.size()==0) {
			return null;
		}
		return path;
	}
	
	/**
	 * Finds all cliques of students in a given school.
	 * 
	 * Returns an array list of array lists - each constituent array list contains
	 * the names of all students in a clique.
	 * 
	 * @param g Graph for which cliques are to be found.
	 * @param school Name of school
	 * @return Array list of clique array lists. Null if there is no student in the
	 *         given school
	 */
	public static ArrayList<ArrayList<String>> cliques(Graph g, String school) {
		
		HashMap <Person,Boolean> alreadyVisited = new HashMap<Person,Boolean>();
		ArrayList<Person> students = new ArrayList<Person>();
		ArrayList<ArrayList<String>> answer = new ArrayList<ArrayList<String>>();
		int size = g.members.length;
		for (int i = 0;i<size;i++) {
			Person current = g.members[i];
			if (current.student == true) {
				if (current.school.equals(school)) {
					students.add(current);
				}
			}
		}
		int studentSize = students.size();
		while(!students.isEmpty()) {
//			System.out.println("pople");
			Person student = students.get(0);
			if (alreadyVisited.get(student)==null) {
//				System.out.println("zzzzzzzz");
				ArrayList<Person> personListFromClicky = getClicky(student,alreadyVisited,g,school);
//				System.out.println(personListFromClicky.size());
				ArrayList<String> temps = new ArrayList<String>();
				while (personListFromClicky.isEmpty()==false) {
//					System.out.println("BEEPBEEPBEEPBEEPBEEPBEEPBEEPBEEPBEEP");
					Person pers = personListFromClicky.get(0);
					temps.add(pers.name);
					alreadyVisited.put(pers,true);
					students.remove(pers);
					personListFromClicky.remove(pers);
				}
//				if (personListFromClicky.isEmpty()==false) {
//					System.out.println("ulululululululu");
//				}
				
				answer.add(temps);
			} else {
				continue;
			}
		}
		
		
		
		
		
		// FOLLOWING LINE IS A PLACEHOLDER TO MAKE COMPILER HAPPY
		// CHANGE AS REQUIRED FOR YOUR IMPLEMENTATION
//		int sizex = answer.size();
//		for (int i = 0;i<sizex;i++) {
//			ArrayList<String> skell = answer.get(i);
//			int sizeSkell = skell.size();
//			System.out.print("[");
//			for (int j=0;j<sizeSkell;j++) {
//				System.out.print(skell.get(j)+",");
//			}
//			System.out.println("]");
//		}
		if (answer.size()==0) {
			return null;
		}
		return answer;
	}
	private static ArrayList<Person> getClicky(Person ps,HashMap<Person,Boolean> alreadyVisited, Graph g, String school){
		
		Friend current = ps.first;
//		Person currentval = g.members[current.fnum];
		ArrayList<Person> clique = new ArrayList<Person>();
		if (alreadyVisited.get(ps)==null) {
			alreadyVisited.put(ps, true);
			clique.add(ps);
		}
//		current = null;
		while (current!=null) {
//			System.out.println("hell");
			Person currentval = g.members[current.fnum];
//			System.out.println(currentval.name);
			if (currentval.student == false || !currentval.school.equals(school)) {
//				System.out.println("crap");
				break;
			}
			if (alreadyVisited.get(currentval) == null) {
//				System.out.println("cll");
				alreadyVisited.put(currentval, true);
				clique.add(currentval);
				clique.addAll(getClicky(currentval,alreadyVisited,g,school));
			}
			current = current.next;
//			current = null;
//			if (current == null) {
//				System.out.println("helterskelter");
		}
		
//		System.out.println("cliquesize "+clique.size());
		
		return clique;
	}
	
	/**
	 * Finds and returns all connectors in the graph.
	 * 
	 * @param g Graph for which connectors needs to be found.
	 * @return Names of all connectors. Null if there are no connectors.
	 */
	public static ArrayList<String> connectors(Graph g) {
		int size = g.members.length;
		HashMap <String,Boolean> alreadyVisited = new HashMap<String,Boolean>();
		int[] numFriends = new int[size];
		ArrayList<String> connectors = new ArrayList<String>();
		for (int i = 0;i<size;i++) {
			int j = 0;
			Friend tes = g.members[i].first;
			while (tes != null){
				tes = tes.next;
				j++;
			}
//			System.out.println(g.members[i].name+":"+j);
			numFriends[i] = j;
		}
		for (int k = 0;k<size;k++) {
			if (numFriends[k]==1) {
				Person connectorFriend = g.members[g.members[k].first.fnum];
				int f = 0;
				Friend ex = connectorFriend.first;
				while (ex!=null) {
					ex = ex.next;
					f++;
				}
				if (f>1) {
					String name = connectorFriend.name;
					if (alreadyVisited.get(name)==null) {
						connectors.add(name);
						alreadyVisited.put(name, true);
					}
					
				}
			}
		}
		
		for (int l = 0;l<connectors.size();l++) {
			System.out.println(connectors.get(l));
		}
		if (connectors.size()==0) {
			return null;
		}
		return connectors;
		
	}
	

//	public static void main(String[] args) {
//		Friends t1 = new Friends();
//		Scanner t1scan = null;
//		try {
//			t1scan = new Scanner(new File("test1.txt"));
//		} catch (FileNotFoundException e) {
//			// TODO Auto-generated catch block
//			e.printStackTrace();
//		}
//		Graph test1 = new Graph(t1scan);
//		Friends.shortestChain(test1,"aparna","rich");
//		Friends.cliques(test1, "rutgers");
//		Friends.connectors(test1);
////		
//		Friends t2 = new Friends();
//		Scanner t2scan = null;
//		try {
//			t2scan = new Scanner(new File("test2.txt"));
//		} catch (FileNotFoundException e) {
//			// TODO Auto-generated catch block
//			e.printStackTrace();
//		}
//		Graph test2 = new Graph(t2scan);
//		Friends.shortestChain(test2,"sam","jane");
//		Friends.cliques(test2, "rutgers");
//		Friends.connectors(test2);
//	}
}
