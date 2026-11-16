-- Plays an animation on the node's AnimationComponent

animationFilepath = ""
speedScale = 1.0
repeatTimes = -1.0 -- Negative means infinite

_kAnimationId = 0
_animationStarted = false

-- Component main update
--- @param info SceneComponentUpdateInfo
function update(info)
	local node = info:getSceneNode()
	local comp = node:tryGetFirstAnimationComponent()
	if comp == nil then
		loge("Failed to get the AnimationComponent")
		return
	end

	if not _animationStarted then
		_animationStarted = true

		comp:setNodeMotionMode(NodeMotionMode.kExtract)
		comp:registerAnimation(_kAnimationId, animationFilepath)
		comp:playAnimation(0, _kAnimationId, "", 0.0, AnimationStart.kNow)
	end

	-- Every frame so the vars can be changed while the animation plays
	if comp:getTrackHasAnimations(0) then
		local state = comp:getAnimationState(0, _kAnimationId)
		state.m_animationSpeedScale = speedScale
		state.m_repeatTimes = repeatTimes
	end

	-- The delta is in parent space
	local delta = comp:getNodeTransformDelta()
	node:setLocalOrigin(node:getLocalOrigin() + delta:getOrigin())
	node:setLocalRotation(delta:getRotation() * node:getLocalRotation())
	node:setLocalScale(node:getLocalScale() * delta:getScale())
end
