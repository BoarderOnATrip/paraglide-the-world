import type { ActivityModeId, ChatMessage, ChatThread } from '../data'

function createMessageId(prefix: string) {
  return `${prefix}-${Date.now()}-${Math.random().toString(36).slice(2, 7)}`
}

function getTimestamp() {
  return new Intl.DateTimeFormat('en-US', {
    hour: '2-digit',
    minute: '2-digit',
    hour12: false,
  }).format(new Date())
}

export function appendOutgoingMessage(
  threads: ChatThread[],
  threadId: string,
  text: string,
) {
  const trimmed = text.trim()

  if (!trimmed) {
    return threads
  }

  return threads.map((thread) =>
    thread.id === threadId
      ? {
          ...thread,
          messages: [
            ...thread.messages,
            {
              id: createMessageId('me'),
              authorId: 'me',
              authorName: 'You',
              text: trimmed,
              timestamp: getTimestamp(),
            },
          ],
        }
      : thread,
  )
}

export function createAutoReply(
  thread: ChatThread | undefined,
  activityModeId: ActivityModeId,
): ChatMessage | null {
  if (!thread) {
    return null
  }

  const replyTextByMode: Record<ActivityModeId, string> = {
    paragliding:
      'Stay light on the keys. The lift looks clean if you want another active run.',
    'scenic-flight':
      'This is a good stretch to talk. The view opens up if we keep drifting east.',
    'boat-tour':
      'Let the water route carry us for a minute. This mode feels great for stories.',
    'car-ride':
      'City ride works here. We can keep talking and switch back to flying later.',
  }

  return {
    id: createMessageId(thread.kind === 'group' ? 'group' : thread.participantIds[0] ?? 'reply'),
    authorId: thread.kind === 'group' ? thread.participantIds[0] ?? 'group' : thread.participantIds[0] ?? 'reply',
    authorName: thread.kind === 'group' ? thread.name : thread.name,
    text: replyTextByMode[activityModeId],
    timestamp: getTimestamp(),
  }
}

export function appendReplyToThread(
  threads: ChatThread[],
  threadId: string,
  reply: ChatMessage,
) {
  return threads.map((thread) =>
    thread.id === threadId
      ? {
          ...thread,
          messages: [...thread.messages, reply],
        }
      : thread,
  )
}
