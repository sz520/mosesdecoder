// $Id$

/***********************************************************************
Moses - factored phrase-based language decoder
Copyright (C) 2006 University of Edinburgh

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
***********************************************************************/

#ifndef moses_ScoreComponentCollection_h
#define moses_ScoreComponentCollection_h

#include <numeric>
#include <cassert>

#include "LMList.h"
#include "ScoreProducer.h"
#include "ScoreIndexManager.h"
#include "FeatureVector.h"
#include "TypeDef.h"
#include "Util.h"

namespace Moses
{

/*** An unweighted collection of scores for a translation or step in a translation.
 *
 * In the factored phrase-based models that are implemented by moses, there are a set of
 * scores that come from a variety of sources (translation probabilities, language model
 * probablilities, distortion probabilities, generation probabilities).  Furthermore, while
 * some of these scores may be 0, this number is fixed (and generally quite small, ie, less
 * than 15), for a given model.
 *
 * The values contained in ScoreComponentCollection objects are unweighted scores (log-probs).
 * 
 * ScoreComponentCollection objects can be added and subtracted, which makes them appropriate
 * to be the datatype used to return the result of a score computations (in this case they will
 * have most values set to zero, except for the ones that are results of the indivudal computation
 * this will then be added into the "running total" in the Hypothesis.  In fact, for a score
 * to be tracked in the hypothesis (and thus to participate in the decoding process), a class
 * representing that score must extend the ScoreProducer abstract base class.  For an example
 * refer to the DistortionScoreProducer class.
 */
class ScoreComponentCollection {
  friend std::ostream& operator<<(std::ostream& os, const ScoreComponentCollection& rhs);
	friend class ScoreIndexManager;
private:
	FVector m_scores;
	const ScoreIndexManager* m_sim;

public:
  //! Create a new score collection with all values set to 0.0
	ScoreComponentCollection();

  //! Clone a score collection
	ScoreComponentCollection(const ScoreComponentCollection& rhs)
	: m_scores(rhs.m_scores)
	, m_sim(rhs.m_sim)
	{}

	inline size_t size() const { return m_scores.size(); }
	float operator[](size_t x) const { return m_scores[m_sim->GetFeatureName(x)]; }

  //! Set all values to 0.0
	void ZeroAll()
	{
	  m_scores.clear();
	}

	void MultiplyEquals(float scalar);
	void MultiplyEquals(const ScoreComponentCollection& rhs);	

  //! add the score in rhs
	void PlusEquals(const ScoreComponentCollection& rhs)
	{
	  m_scores += rhs.m_scores;
	}

  //! subtract the score in rhs
	void MinusEquals(const ScoreComponentCollection& rhs)
	{
	  m_scores -= rhs.m_scores;
	}

	//! Add scores from a single ScoreProducer only
	//! The length of scores must be equal to the number of score components
	//! produced by sp
	void PlusEquals(const ScoreProducer* sp, const std::vector<float>& scores);

	//! Add scores from a single ScoreProducer only
	//! The length of scores must be equal to the number of score components
	//! produced by sp
	void PlusEquals(const ScoreProducer* sp, const ScoreComponentCollection& scores)
	{
		size_t i = m_sim->GetBeginIndex(sp->GetScoreBookkeepingID());
		const size_t end = m_sim->GetEndIndex(sp->GetScoreBookkeepingID());
		for (; i < end; ++i)
		{
			m_scores[m_sim->GetFeatureName(i)] += scores.m_scores[m_sim->GetFeatureName(i)];
		}  
	}

	//! Special version PlusEquals(ScoreProducer, vector<float>)
	//! to add the score from a single ScoreProducer that produces
	//! a single value
	void PlusEquals(const ScoreProducer* sp, float score)
	{
		assert(1 == sp->GetNumScoreComponents());
		const size_t i = m_sim->GetBeginIndex(sp->GetScoreBookkeepingID());
		m_scores[m_sim->GetFeatureName(i)] += score;
	}

	void Assign(const ScoreProducer* sp, const std::vector<float>& scores)
	{
		assert(scores.size() == sp->GetNumScoreComponents());
		size_t i = m_sim->GetBeginIndex(sp->GetScoreBookkeepingID());
		for (std::vector<float>::const_iterator vi = scores.begin();
		     vi != scores.end(); ++vi)
		{
			m_scores[m_sim->GetFeatureName(i++)] = *vi;
		}  
	}

	void Assign(const ScoreComponentCollection &copy)
	{
		m_scores =  copy.m_scores;
	}
	
	//! Special version PlusEquals(ScoreProducer, vector<float>)
	//! to add the score from a single ScoreProducer that produces
	//! a single value
	void Assign(const ScoreProducer* sp, float score)
	{
		assert(1 == sp->GetNumScoreComponents());
		const size_t i = m_sim->GetBeginIndex(sp->GetScoreBookkeepingID());
		m_scores[m_sim->GetFeatureName(i)] = score;
	}

	void Assign(size_t index, float score)
	{
		m_scores[m_sim->GetFeatureName(index)] = score;
	}

  //! Used to find the weighted total of scores.  rhs should contain a vector of weights
  //! of the same length as the number of scores.
	float InnerProduct(const std::vector<float>& rhs) const
	{
		return m_scores.inner_product(rhs);
	}

	float InnerProduct(const ScoreComponentCollection& rhs) const
	{
		return m_scores.inner_product(rhs.m_scores);
	}
	
	float PartialInnerProduct(const ScoreProducer* sp, const std::vector<float>& rhs) const
	{
		std::vector<float> lhs = GetScoresForProducer(sp);
		assert(lhs.size() == rhs.size());
		return std::inner_product(lhs.begin(), lhs.end(), rhs.begin(), 0.0f);
	}

	//! return a vector of all the scores associated with a certain ScoreProducer
	std::vector<float> GetScoresForProducer(const ScoreProducer* sp) const
	{
		size_t id = sp->GetScoreBookkeepingID();
		const size_t begin = m_sim->GetBeginIndex(id);
		const size_t end = m_sim->GetEndIndex(id);
		std::vector<float> res(end-begin);
		size_t j = 0;
		for (size_t i = begin; i < end; i++) {
			res[j++] = m_scores[m_sim->GetFeatureName(i)];
		}
		return res;  
	}

	//! if a ScoreProducer produces a single score (for example, a language model score)
	//! this will return it.  If not, this method will throw
	float GetScoreForProducer(const ScoreProducer* sp) const
	{
		size_t id = sp->GetScoreBookkeepingID();
		const size_t begin = m_sim->GetBeginIndex(id);
#ifndef NDEBUG
		const size_t end = m_sim->GetEndIndex(id);
		assert(end-begin == 1);
#endif
		return m_scores[m_sim->GetFeatureName(begin)];
	}

	float GetWeightedScore() const;

    void ZeroAllLM(const LMList& lmList);
    void PlusEqualsAllLM(const LMList& lmList, const ScoreComponentCollection& rhs);

};

}
#endif
