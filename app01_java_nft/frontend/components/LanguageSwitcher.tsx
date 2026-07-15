'use client';

import { useTranslation } from '../lib/i18n/context';

const LANGUAGES: { code: 'en' | 'pl'; flag: string; label: string }[] = [
  { code: 'en', flag: '🇬🇧', label: 'EN' },
  { code: 'pl', flag: '🇵🇱', label: 'PL' },
];

export function LanguageSwitcher() {
  const { lang, setLang } = useTranslation();

  return (
    <div className="flex items-center gap-1" role="group" aria-label="Language">
      {LANGUAGES.map((option) => (
        <button
          key={option.code}
          type="button"
          onClick={() => setLang(option.code)}
          aria-pressed={lang === option.code}
          className={`flex items-center gap-1 rounded-full px-2 py-1 text-sm ${
            lang === option.code ? 'bg-ink/10 font-semibold' : 'text-ink/60 hover:bg-ink/5'
          }`}
        >
          <span aria-hidden>{option.flag}</span>
          {option.label}
        </button>
      ))}
    </div>
  );
}
