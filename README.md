# a simple inverted index

inverted_index stores a collection of documents, where each document is a sequences of terms. 

Each document is stored with a unique id. 

The match function efficiently finds the document or set of documents that is most similar to the collection of search terms (a document itself) provided.

The class template can be configured to store additional data for each document term and a custom score function can be provided to define document similarity.

# knn

The knn class is a k nearest neighbor classfier that uses the inverted_index for search and storage. As such is it suited for discrete valued data vectors.

input to the learn method of the classifier are data vectors and their labels. 

classify takes a data vector and return a label based on the k nearst neighbors. By default the scores of the neighbors are combined using a majority vote, but a custom combination function can be provided.
