#ifndef __INVERTED_INDEX_HPP__
#define __INVERTED_INDEX_HPP__

#include <map>
#include <deque>

#include "utilities.hpp"


// Postings may contain internal data
// By default only the position of a term in the document(vector) is stored.
// if the same term appears multiple times in a document and CombinePositingsPerTerm == true
// the update function will be called. The default behavior is to store only the position of the first occurence of the term
struct NoPostingData {
	template<typename Term, typename DocumentId>
	NoPostingData(DocumentId& id, size_t position, const Term& term) {}

	template<typename Term, typename DocumentId>
	void update(DocumentId& id, size_t position, const Term& term) {}
}; //NoPostingData


// Default ScoreFunction: count the number of matches
// The score function receives the previous score, the term, the posting and the total number of postings for this term
// The latter may for example be used to normalize scores.
//
// The ouput type of the score function should match the Score parameter of InvertedIndex<>
struct number_of_matches {
	template<typename Posting, typename Term>
	int operator()(int old_score, const Term& term, const Posting& posting, size_t num_postings) const {
		return ++old_score;
	}
}; // number_of_matches


template <
	typename Term = unsigned, 
	typename DocumentId = unsigned,
	typename PostingData = NoPostingData,
	bool CombinePostingsPerTerm = true,
	typename Score = int
>
class InvertedIndex {
public:
	typedef std::pair<DocumentId, Score> Match;

	class Posting : public PostingData {
		DocumentId _id;
	public:
		Posting(const DocumentId& id, size_t position, const Term& term)
			: PostingData(id, position, term), _id(id) 
		{}
		
		DocumentId id() const {return _id;}
	}; // Posting

private:
	typedef std::deque<Posting> Postings;

	typedef std::map<Term, Postings> Index;

public:
	/*----- Accumulator -----------------------------------------------------*/

	template<typename ScoreFunction>
	class Accumulator {
	private:
		typedef std::map<DocumentId, Score> Accu;
		typedef std::multimap<Score, DocumentId> Ranking;

		Accu _accu;
		const ScoreFunction& _score;
	public:
		Accumulator(const ScoreFunction& score) : _score(score) {}

		void update(const Term& term, const Posting& posting, size_t num_postings) {
			_accu[posting.id()] = _score(_accu[posting.id()], term, posting, num_postings);
	  }

		template<typename OutputFunction>
		void matches(OutputFunction& out) {
			for (auto a_itr = _accu.begin(); a_itr != _accu.end(); ++a_itr) {
				out(*a_itr);
			} // for
		}

		template<typename OutputFunction>
		void matches(OutputFunction& out, int k) { // NOTE: Boost::MultiIndex might be more efficient
			if (k == 0) {
			  matches(out);
			}
			else {
			  Ranking ranking;

			  for (auto a_itr = _accu.begin(); a_itr != _accu.end(); ++a_itr) {
					ranking.insert(swap_pair(*a_itr));
			  } // for


			  int i = 0;
			  for (auto r_itr = ranking.rbegin(); r_itr != ranking.rend() && i != k; ++i, ++r_itr) {
			    out(swap_pair(*r_itr));
			  } // for
			} // else
		}

		typename InvertedIndex::Match best_match() {
			if (_accu.empty())
				throw "No matching documents";

			auto best_itr = _accu.begin();

			for (auto a_itr = _accu.begin(); a_itr != _accu.end(); ++a_itr) {
				if (best_itr->second < a_itr->second) {
					best_itr = a_itr;
				}
			} // for
			
			return *best_itr;
		}
	}; // Accumulator


private:
	Index _index;

	// CombinePostingsPerTerm == false
	void insert_posting(const DocumentId& id, size_t pos, const Term& term, BoolType<false>) {
	  _index[term].push_back(Posting(id, pos, term));
	}

	// CombinePostingsPerTerm == true
	void insert_posting(const DocumentId& id, size_t position, const Term& term, BoolType<true>) {
    Postings& postings = _index[term];

	  if (!postings.empty() && postings.back().id() == id) {
			postings.back().update(id, position, term);
	  }
	  else {
			postings.push_back(Posting(id, position, term));
	  } // else
	}

public:
	template<typename AccumulatorT>
	void match_term(const Term& term, AccumulatorT& accumulator) {
	  auto term_itr = _index.find(term);

	  if (term_itr != _index.end()) {
	    for (auto p_itr = term_itr->second.begin(); p_itr != term_itr->second.end(); ++p_itr) {
		  	accumulator.update(term_itr->first, *p_itr, term_itr->second.size());
			} // for
	  } // if
	}

	void insert_posting(const DocumentId& id, size_t pos, const Term& term) {
		insert_posting(id, pos, term, BoolType<CombinePostingsPerTerm>());
	}

	template<typename Iterator>
	const DocumentId& insert(const DocumentId& id, Iterator first, Iterator last) {
		for (size_t pos = 0; first != last; ++first, ++pos) {
		  insert_posting(id, pos, *first);
		} // for

		return id;
	}

	template<typename Iterator, typename ScoreFunction>
	Match best_match(Iterator first, Iterator last, const ScoreFunction& score) {
		Accumulator<ScoreFunction> accumulator(score);

		for ( ; first != last; ++first) {
		  match_term(*first, accumulator);
		} // for

		return accumulator.best_match();
	}

	// find the k best matches. The OutputFunction functor will be called for each of these results, if k = 0, all results will added to the output
	template<typename InputIterator, typename ScoreFunction, typename OutputFunction>
	void match(InputIterator first, InputIterator last, const ScoreFunction& score, OutputFunction& out, int k) {  // Note: might want to use pass by value, at least for OutputFunction
		Accumulator<ScoreFunction> accumulator(score);

		for ( ; first != last; ++first) {
		  match_term(*first, accumulator);
		} // for

		accumulator.matches(out, k);
	}
}; // InvertedIndex

#endif // __INVERTED_INDEX_HPP__
