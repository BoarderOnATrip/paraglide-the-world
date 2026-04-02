import {
  COUNTRY_CONTENT,
  FINGER_GUIDES as COUNTRY_FINGER_GUIDES,
} from '../data/countries'
import type {
  CountryContent as BaseCountryContent,
  FingerGuide as BaseFingerGuide,
} from '../data/content-types'

export type CountryId =
  | 'brazil'
  | 'japan'
  | 'kenya'
  | 'italy'
  | 'switzerland'
  | 'turkey'

export type CountryContent = BaseCountryContent & {
  id: CountryId
}

export type FingerGuideId =
  | 'lp'
  | 'lr'
  | 'lm'
  | 'li'
  | 'ri'
  | 'rm'
  | 'rr'
  | 'rp'
  | 'th'

export type FingerGuide = BaseFingerGuide & {
  id: FingerGuideId
}

export const COUNTRIES = COUNTRY_CONTENT as readonly CountryContent[]
export const FINGER_GUIDES = COUNTRY_FINGER_GUIDES as readonly FingerGuide[]

export const KEY_TO_FINGER: Readonly<Record<string, FingerGuide>> = {
  q: FINGER_GUIDES[0],
  a: FINGER_GUIDES[0],
  z: FINGER_GUIDES[0],
  w: FINGER_GUIDES[1],
  s: FINGER_GUIDES[1],
  x: FINGER_GUIDES[1],
  e: FINGER_GUIDES[2],
  d: FINGER_GUIDES[2],
  c: FINGER_GUIDES[2],
  r: FINGER_GUIDES[3],
  f: FINGER_GUIDES[3],
  v: FINGER_GUIDES[3],
  t: FINGER_GUIDES[3],
  g: FINGER_GUIDES[3],
  b: FINGER_GUIDES[3],
  y: FINGER_GUIDES[4],
  h: FINGER_GUIDES[4],
  n: FINGER_GUIDES[4],
  u: FINGER_GUIDES[4],
  j: FINGER_GUIDES[4],
  m: FINGER_GUIDES[4],
  i: FINGER_GUIDES[5],
  k: FINGER_GUIDES[5],
  o: FINGER_GUIDES[6],
  l: FINGER_GUIDES[6],
  p: FINGER_GUIDES[7],
  ' ': FINGER_GUIDES[8],
}

export function getCountryContent(countryId: CountryId) {
  return COUNTRIES.find((country) => country.id === countryId) ?? COUNTRIES[0]
}

export function getLessonText(country: CountryContent) {
  return country.lessonWords.join(' ')
}

export function getFingerGuide(character: string) {
  return KEY_TO_FINGER[character.toLowerCase()] ?? FINGER_GUIDES[8]
}
