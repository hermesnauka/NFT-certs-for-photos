'use client';

import { useTranslation } from '../lib/i18n/context';

/** Renders a single translated string inline — lets server-component pages embed i18n text
 * without becoming client components themselves. */
export function T({ k, fallback }: { k: string; fallback: string }) {
  const { t } = useTranslation();
  return <>{t(k, fallback)}</>;
}
