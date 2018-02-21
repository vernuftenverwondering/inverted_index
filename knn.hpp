#ifndef __KNN_HPP__
#define __KNN_HPP__

#include "inverted_index.hpp"

// A simple k-nearest neighbor classifier for discrete valued data vectors based on an inverted index.
// The keys to the index are pairs of the position in the data vector and its value.

template<typename Label = int>
class KNN {
private:
	typedef std::pair<size_t, int> Key;
	typedef InvertedIndex<Key, int, NoPostingData, false> Index;

	Index _index;
	std::vector<Label> _labels;

	class ToKey {
	private:
		size_t _position;
	public:
		ToKey() : _position(0) {}

		Key operator()(int feature) {
			return std::make_pair(_position++, feature);
		}
	}; // ToKey

	template<typename Iterator>
    std::vector<Key> to_terms(Iterator first, Iterator last) {
		  std::vector<Key> terms;
		
		  for (int position = 0; first != last; ++first, ++position) {
		    terms.push_back(std::make_pair(position, *first));
		  } // for

		  return terms;
    }
public:
	template<typename Iterator>
	void learn(Iterator first, Iterator last, const Label& label) {
		
		_labels.push_back(label);

		auto terms = to_terms(first, last);
		_index.insert(_labels.size() - 1, terms.begin(), terms.end());
	}

	// find the label of the nearest neighbor (1-nn)
	template<typename Iterator>
	Label classify(Iterator first, Iterator last) {
		auto terms = to_terms(first, last);
		return _labels[_index.best_match(terms.begin(), terms.end(), number_of_matches()).first];
	}

	// find the label based on k nearest neighbors using a custom function to combine the k scores
	// input for the combination function are the counts per label
	template<typename Iterator, typename CombinationFunction>
	Label classify(Iterator first, Iterator last, unsigned k, const CombinationFunction& combine) {
		int counts_per_label;

		auto add_counts = [&](const std::pair<int, int>& score) {
			++counts_per_label[this->_labels[score.first]];
		};

		_index.match(first, last, number_of_matches(), add_counts, k);

		return combine(counts_per_label.begin(), counts_per_label.end());
	}

	// find the best label based on k nearest neighbors using majority voting
	template<typename Iterator>
	Label classify(Iterator first, Iterator last, unsigned k) {
		struct marjority_vote {
			Label operator()(Iterator first, Iterator last) const {
				Iterator best = first;
				for ( ; first != last; ++first) {
					if (best->second < first->second)
						best = first;
				} // for

				return best->first;
			}
		}; // majority_vote

		return classify(first, last, k, majority_vote());
	}
}; // KNN

#endif // __KNN_HPP__

