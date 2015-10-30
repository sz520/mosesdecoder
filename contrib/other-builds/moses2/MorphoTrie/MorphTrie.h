#ifndef MORPHTRIE_H_
#define MORPHTRIE_H_

#include <vector>
#include "Node.h"

template<class KeyClass, class ValueClass>
class MorphTrie
{
    public:
        MorphTrie() {}
        Node<KeyClass, ValueClass>* insert(const std::vector<KeyClass>& word, const ValueClass& value);
        const Node<KeyClass, ValueClass>* getNode(const std::vector<KeyClass>& words) const;
        const Node<KeyClass, ValueClass> &getNode(const std::vector<KeyClass>& words, size_t &stoppedAtInd) const;
    private:
        Node<KeyClass, ValueClass> root;
};

template<class KeyClass, class ValueClass>
Node<KeyClass, ValueClass>* MorphTrie<KeyClass, ValueClass>::insert(const std::vector<KeyClass>& word,
																	const ValueClass& value)
{
    Node<KeyClass, ValueClass>* cNode = &root;
    for (size_t i = 0; i < word.size(); ++i)
    {
        KeyClass cKey = word[i];
        cNode = cNode->addSubnode(cKey);
    }
    cNode->setValue(value);
    return cNode;
}

template<class KeyClass, class ValueClass>
const Node<KeyClass, ValueClass>* MorphTrie<KeyClass, ValueClass>::getNode(const std::vector<KeyClass>& words) const
{
	size_t stoppedAtInd;
	Node<KeyClass, ValueClass> &ret = getNode(words, stoppedAtInd);
	if (stoppedAtInd < words.size()) {
		ret = NULL;
	}
	return &ret;
}

template<class KeyClass, class ValueClass>
const Node<KeyClass, ValueClass> &MorphTrie<KeyClass, ValueClass>::getNode(const std::vector<KeyClass>& words, size_t &stoppedAtInd) const
{
    const Node<KeyClass, ValueClass> *prevNode = &root, *newNode;
    for (size_t i = 0; i < words.size(); ++i)
    {
        const KeyClass &cKey = words[i];
        newNode = prevNode->findSub(cKey);
        if (newNode == NULL)
        {
        	stoppedAtInd = i;
            return *prevNode;
        }
        prevNode = newNode;
    }

    stoppedAtInd = words.size();
    return *newNode;
}


#endif /* end of include guard: MORPHTRIE_H_ */