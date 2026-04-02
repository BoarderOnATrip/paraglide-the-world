export type GoogleWorldMode = 'standard-2d' | 'premium-3d'

export const DEFAULT_GOOGLE_WORLD_MODE: GoogleWorldMode = 'standard-2d'

export const GOOGLE_WORLD_MODE_OPTIONS: Array<{
  id: GoogleWorldMode
  label: string
  summary: string
}> = [
  {
    id: 'standard-2d',
    label: 'Standard 2D',
    summary: 'Google satellite tiles with the same flight model and chase camera.',
  },
  {
    id: 'premium-3d',
    label: 'Premium 3D',
    summary: 'Google photorealistic 3D tiles for the full Earth-flying presentation.',
  },
]
