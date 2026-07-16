import { DashboardView } from '../components/DashboardView';
import { T } from '../components/T';

export default function DashboardPage() {
  return (
    <div className="space-y-8">
      <div>
        <h1 className="font-serif text-3xl font-semibold">
          <T k="dashboard.title" fallback="My Certificates Dashboard" />
        </h1>
        <p className="mt-1 text-ink/60">
          Certificates of authenticity minted to your connected wallet.
        </p>
      </div>
      <DashboardView />
    </div>
  );
}
