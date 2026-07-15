import { MintWizard } from '../../components/MintWizard';
import { T } from '../../components/T';

export default function MintPage() {
  return (
    <div className="space-y-8">
      <div>
        <h1 className="font-serif text-3xl font-semibold">
          <T k="mint.title" fallback="Mint a Certificate" />
        </h1>
        <p className="mt-1 text-ink/60">
          <T
            k="mint.subtitle"
            fallback="Turn a photograph into a verifiable certificate of authenticity."
          />
        </p>
      </div>
      <MintWizard />
    </div>
  );
}
